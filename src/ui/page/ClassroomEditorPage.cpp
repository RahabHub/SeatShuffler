/**
 * @file ClassroomEditorPage.cpp
 * @brief 教室布局编辑页面实现
 *
 * 修复内容:
 *   - 增加脏标记 (m_dirty) 追踪，保存后返回不再重复询问
 *   - 文本编辑和行列变化均会触发脏标记
 *   - 按钮样式适配全局 AppStyle
 */

#include "ClassroomEditorPage.h"
#include <QButtonGroup>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <utility>
#include "MainWindow.h"
#include "src/storage/DataManager.h"
#include "src/ui/common/AppStyle.h"
#include "src/ui/widgets/ClassroomGridWidget.h"

namespace seat {

    ClassroomEditorPage::ClassroomEditorPage(QString classroomId, MainWindow *mainWindow, QWidget *parent) :
        QWidget(parent), m_mainWindow(mainWindow), m_classroomId(std::move(classroomId)) {
        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(20, 16, 20, 16);
        layout->setSpacing(12);

        // ─── 顶部: 返回 + 保存 ─────────────────────
        auto *topBar = new QHBoxLayout();
        auto *btnBack = new QPushButton("← 返回");
        btnBack->setCursor(Qt::PointingHandCursor);
        connect(btnBack, &QPushButton::clicked, this, &ClassroomEditorPage::onBack);
        topBar->addWidget(btnBack);
        topBar->addStretch();

        auto *btnSave = new QPushButton("保存");
        AppStyle::applyPrimaryStyle(btnSave);
        btnSave->setMinimumWidth(80);
        btnSave->setCursor(Qt::PointingHandCursor);
        connect(btnSave, &QPushButton::clicked, this, &ClassroomEditorPage::onSave);
        topBar->addWidget(btnSave);
        layout->addLayout(topBar);

        // ─── 名称 & 备注 ───────────────────────────
        auto *metaLayout = new QHBoxLayout();
        metaLayout->addWidget(new QLabel("教室名称:"));
        m_nameEdit = new QLineEdit();
        m_nameEdit->setPlaceholderText("请输入教室名称");
        metaLayout->addWidget(m_nameEdit, 1);
        metaLayout->addSpacing(16);
        metaLayout->addWidget(new QLabel("备注:"));
        m_noteEdit = new QTextEdit();
        m_noteEdit->setMaximumHeight(50);
        m_noteEdit->setPlaceholderText("可选备注信息");
        metaLayout->addWidget(m_noteEdit, 2);
        layout->addLayout(metaLayout);

        // 名称和备注变化时标记为脏
        connect(m_nameEdit, &QLineEdit::textChanged, this, &ClassroomEditorPage::markDirty);
        connect(m_noteEdit, &QTextEdit::textChanged, this, &ClassroomEditorPage::markDirty);

        // ─── 控制栏: 行列数 + 画刷选择 ─────────────
        auto *controlBar = new QHBoxLayout();

        controlBar->addWidget(new QLabel("行数:"));
        m_rowsSpin = new QSpinBox();
        m_rowsSpin->setRange(1, 30);
        m_rowsSpin->setValue(6);
        connect(m_rowsSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ClassroomEditorPage::onRowsChanged);
        controlBar->addWidget(m_rowsSpin);

        controlBar->addSpacing(10);
        controlBar->addWidget(new QLabel("列数:"));
        m_colsSpin = new QSpinBox();
        m_colsSpin->setRange(1, 30);
        m_colsSpin->setValue(8);
        connect(m_colsSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ClassroomEditorPage::onColsChanged);
        controlBar->addWidget(m_colsSpin);

        controlBar->addSpacing(30);

        // 画刷选择
        auto *brushGroup = new QGroupBox("画刷工具 (点击/拖拽网格)");
        auto *brushLayout = new QHBoxLayout(brushGroup);

        auto *rbSeat = new QRadioButton("座位");
        auto *rbAisle = new QRadioButton("过道");
        auto *rbEmpty = new QRadioButton("空位");
        rbAisle->setChecked(true);

        auto *btnGroup = new QButtonGroup(this);
        btnGroup->addButton(rbSeat, 0);
        btnGroup->addButton(rbAisle, 1);
        btnGroup->addButton(rbEmpty, 2);

        brushLayout->addWidget(rbSeat);
        brushLayout->addWidget(rbAisle);
        brushLayout->addWidget(rbEmpty);

        controlBar->addWidget(brushGroup);
        controlBar->addStretch();

        // 座位数统计标签
        auto *seatCountLabel = new QLabel();
        seatCountLabel->setStyleSheet("color: #1677ff; font-weight: bold; font-size: 13px;");
        controlBar->addWidget(seatCountLabel);

        layout->addLayout(controlBar);

        // ─── 网格编辑器 (带滚动) ────────────────────
        auto *scrollArea = new QScrollArea();
        scrollArea->setWidgetResizable(true);

        m_gridWidget = new ClassroomGridWidget();
        scrollArea->setWidget(m_gridWidget);
        layout->addWidget(scrollArea, 1);

        // 画刷切换连接
        connect(btnGroup, &QButtonGroup::idClicked, this, [this](int id) {
            switch (id) {
                case 0:
                    m_gridWidget->setBrush(ClassroomGridWidget::BrushType::Seat);
                    break;
                case 1:
                    m_gridWidget->setBrush(ClassroomGridWidget::BrushType::Aisle);
                    break;
                case 2:
                    m_gridWidget->setBrush(ClassroomGridWidget::BrushType::Empty);
                    break;
            }
        });

        // 网格修改时更新座位数并标记为脏
        connect(m_gridWidget, &ClassroomGridWidget::gridModified, this, [this, seatCountLabel]() {
            markDirty();
            if (m_gridWidget->classroom()) {
                seatCountLabel->setText(QString("座位数: %1").arg(m_gridWidget->classroom()->seatCount()));
            }
        });

        // 加载数据
        loadData();

        // 初始化座位数显示
        seatCountLabel->setText(QString("座位数: %1").arg(m_classroom.seatCount()));

        // 加载完成后重置脏标记 (loadData 过程中会触发信号)
        m_dirty = false;
    }

    void ClassroomEditorPage::loadData() {
        m_classroom = DataManager::instance().loadClassroom(m_classroomId);
        m_nameEdit->setText(m_classroom.name);
        m_noteEdit->setPlainText(m_classroom.note);
        m_rowsSpin->setValue(m_classroom.rows);
        m_colsSpin->setValue(m_classroom.cols);
        m_gridWidget->setClassroom(&m_classroom);
    }

    void ClassroomEditorPage::markDirty() { m_dirty = true; }

    void ClassroomEditorPage::onBack() {
        // 仅在有未保存修改时询问用户
        if (m_dirty) {
            const auto ret = QMessageBox::question(this, "返回", "有未保存的修改，是否保存？",
                                                   QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
            if (ret == QMessageBox::Cancel)
                return;
            if (ret == QMessageBox::Yes)
                onSave();
        }
        m_mainWindow->popPage();
    }

    void ClassroomEditorPage::onSave() {
        m_classroom.name = m_nameEdit->text().trimmed();
        m_classroom.note = m_noteEdit->toPlainText().trimmed();

        if (m_classroom.name.isEmpty()) {
            QMessageBox::warning(this, "提示", "教室名称不能为空");
            return;
        }

        DataManager::instance().saveClassroom(m_classroom);

        // 保存成功后清除脏标记
        m_dirty = false;

        QMessageBox::information(this, "保存成功", "教室数据已保存");
    }

    void ClassroomEditorPage::onRowsChanged(int value) {
        m_classroom.resizeGrid(value, m_classroom.cols);
        markDirty();
        m_gridWidget->update();
    }

    void ClassroomEditorPage::onColsChanged(int value) {
        m_classroom.resizeGrid(m_classroom.rows, value);
        markDirty();
        m_gridWidget->update();
    }

} // namespace seat
