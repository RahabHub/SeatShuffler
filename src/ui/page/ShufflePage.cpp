/**
 * @file ShufflePage.cpp
 * @brief 正式抽取页面实现
 *
 * 用户选择人员组、教室、规则后执行座位抽取，
 * 结果以网格形式展示，可保存历史、导出图片和表格。
 */

#include "ShufflePage.h"
#include "HistoryPage.h"
#include "MainWindow.h"
#include "src/engine/ShuffleEngine.h"
#include "src/storage/DataManager.h"
#include "src/ui/common/AppStyle.h"
#include "src/ui/widgets/ResultGridWidget.h"

#include <QApplication>
#include <QFile>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QTextStream>
#include <QVBoxLayout>

namespace seat {

    ShufflePage::ShufflePage(MainWindow *mainWindow, QWidget *parent) : QWidget(parent), m_mainWindow(mainWindow) {
        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(24, 20, 24, 20);
        layout->setSpacing(14);

        // ─── 标题 ───────────────────────────────────
        auto *headerLayout = new QHBoxLayout();
        auto *titleLabel = new QLabel("座位抽取");
        titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #1f1f1f; background: transparent;");
        headerLayout->addWidget(titleLabel);
        headerLayout->addStretch();

        auto *btnHistory = new QPushButton("历史记录");
        btnHistory->setCursor(Qt::PointingHandCursor);
        connect(btnHistory, &QPushButton::clicked, this, &ShufflePage::onViewHistory);
        headerLayout->addWidget(btnHistory);
        layout->addLayout(headerLayout);

        // ─── 选择区域 ──────────────────────────────
        auto *selectGroup = new QGroupBox("选择参数");
        auto *selectLayout = new QHBoxLayout(selectGroup);
        selectLayout->setSpacing(16);

        // 人员组
        auto *groupBox = new QVBoxLayout();
        auto *groupLabel = new QLabel("人员组:");
        groupLabel->setStyleSheet("font-weight: bold; background: transparent;");
        groupBox->addWidget(groupLabel);
        m_groupCombo = new QComboBox();
        m_groupCombo->setMinimumWidth(160);
        groupBox->addWidget(m_groupCombo);
        selectLayout->addLayout(groupBox);

        // 姓名字段
        auto *nameBox = new QVBoxLayout();
        auto *nameFieldLabel = new QLabel("姓名字段:");
        nameFieldLabel->setStyleSheet("font-weight: bold; background: transparent;");
        nameBox->addWidget(nameFieldLabel);
        m_nameFieldCombo = new QComboBox();
        m_nameFieldCombo->setMinimumWidth(100);
        nameBox->addWidget(m_nameFieldCombo);
        selectLayout->addLayout(nameBox);

        connect(m_groupCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
            m_nameFieldCombo->clear();
            if (index < 0)
                return;
            QString groupId = m_groupCombo->currentData().toString();
            auto group = DataManager::instance().loadGroup(groupId);
            m_nameFieldCombo->addItem("(使用ID)", "");
            for (const auto &col: group.columns) {
                m_nameFieldCombo->addItem(col.name, col.name);
            }
            if (!group.columns.isEmpty()) {
                m_nameFieldCombo->setCurrentIndex(1);
            }
        });

        // 教室
        auto *classroomBox = new QVBoxLayout();
        auto *classroomLabel = new QLabel("教室:");
        classroomLabel->setStyleSheet("font-weight: bold; background: transparent;");
        classroomBox->addWidget(classroomLabel);
        m_classroomCombo = new QComboBox();
        m_classroomCombo->setMinimumWidth(160);
        classroomBox->addWidget(m_classroomCombo);
        selectLayout->addLayout(classroomBox);

        // 规则
        auto *ruleBox = new QVBoxLayout();
        auto *ruleLabel = new QLabel("规则:");
        ruleLabel->setStyleSheet("font-weight: bold; background: transparent;");
        ruleBox->addWidget(ruleLabel);
        m_ruleCombo = new QComboBox();
        m_ruleCombo->setMinimumWidth(160);
        ruleBox->addWidget(m_ruleCombo);
        selectLayout->addLayout(ruleBox);

        layout->addWidget(selectGroup);

        // ─── 操作按钮 ──────────────────────────────
        auto *actionBar = new QHBoxLayout();

        auto *btnShuffle = new QPushButton("开始抽取");
        AppStyle::applyPrimaryStyle(btnShuffle);
        btnShuffle->setMinimumHeight(38);
        btnShuffle->setMinimumWidth(130);
        btnShuffle->setCursor(Qt::PointingHandCursor);
        QFont btnFont = btnShuffle->font();
        btnFont.setBold(true);
        btnFont.setPointSize(13);
        btnShuffle->setFont(btnFont);
        connect(btnShuffle, &QPushButton::clicked, this, &ShufflePage::onShuffle);
        actionBar->addWidget(btnShuffle);

        auto *btnSave = new QPushButton("保存结果");
        btnSave->setCursor(Qt::PointingHandCursor);
        connect(btnSave, &QPushButton::clicked, this, &ShufflePage::onSaveRecord);
        actionBar->addWidget(btnSave);

        auto *btnExportImg = new QPushButton("导出图片");
        btnExportImg->setCursor(Qt::PointingHandCursor);
        connect(btnExportImg, &QPushButton::clicked, this, &ShufflePage::onExportImage);
        actionBar->addWidget(btnExportImg);

        auto *btnExportCsv = new QPushButton("导出表格");
        btnExportCsv->setCursor(Qt::PointingHandCursor);
        connect(btnExportCsv, &QPushButton::clicked, this, &ShufflePage::onExportCsv);
        actionBar->addWidget(btnExportCsv);

        actionBar->addStretch();

        m_statusLabel = new QLabel();
        m_statusLabel->setStyleSheet("color: #8c8c8c; font-size: 13px; background: transparent;");
        actionBar->addWidget(m_statusLabel);

        layout->addLayout(actionBar);

        // ─── 结果显示区域 (可滚动) ─────────────────
        auto *scrollArea = new QScrollArea();
        scrollArea->setWidgetResizable(true);

        m_resultGrid = new ResultGridWidget();
        scrollArea->setWidget(m_resultGrid);
        layout->addWidget(scrollArea, 1);
    }

    void ShufflePage::refresh() { populateCombos(); }

    void ShufflePage::populateCombos() const {
        const QString prevGroup = m_groupCombo->currentData().toString();
        const QString prevClassroom = m_classroomCombo->currentData().toString();
        const QString prevRule = m_ruleCombo->currentData().toString();

        m_groupCombo->clear();
        m_classroomCombo->clear();
        m_ruleCombo->clear();

        auto groups = DataManager::instance().loadAllGroups();
        for (const auto &g: groups)
            m_groupCombo->addItem(QString("%1 (%2人)").arg(g.name).arg(g.persons.size()), g.id);

        auto rooms = DataManager::instance().loadAllClassrooms();
        for (const auto &r: rooms)
            m_classroomCombo->addItem(QString("%1 (%2座)").arg(r.name).arg(r.seatCount()), r.id);

        m_ruleCombo->addItem("(无规则 - 纯随机)", "");
        auto rules = DataManager::instance().loadAllRules();
        for (const auto &r: rules)
            m_ruleCombo->addItem(QString("%1 (%2条约束)").arg(r.name).arg(r.constraints.size()), r.id);

        auto restoreCombo = [](QComboBox *combo, const QString &val) {
            if (!val.isEmpty()) {
                int idx = combo->findData(val);
                if (idx >= 0)
                    combo->setCurrentIndex(idx);
            }
        };
        restoreCombo(m_groupCombo, prevGroup);
        restoreCombo(m_classroomCombo, prevClassroom);
        restoreCombo(m_ruleCombo, prevRule);
    }

    // ============================================================
    // 抽取
    // ============================================================

    void ShufflePage::onShuffle() {
        if (m_groupCombo->currentIndex() < 0) {
            QMessageBox::warning(this, "提示", "请先选择一个人员组");
            return;
        }
        if (m_classroomCombo->currentIndex() < 0) {
            QMessageBox::warning(this, "提示", "请先选择一个教室");
            return;
        }

        QString groupId = m_groupCombo->currentData().toString();
        QString classroomId = m_classroomCombo->currentData().toString();
        QString ruleId = m_ruleCombo->currentData().toString();
        QString nameField = m_nameFieldCombo->currentData().toString();

        auto group = DataManager::instance().loadGroup(groupId);
        auto room = DataManager::instance().loadClassroom(classroomId);

        Rule rule;
        if (!ruleId.isEmpty()) {
            rule = DataManager::instance().loadRule(ruleId);
        } else {
            rule.id = "";
            rule.name = "无规则";
        }

        m_statusLabel->setText("正在抽取...");
        m_statusLabel->setStyleSheet("color: #1677ff; font-size: 13px; background: transparent;");
        QApplication::processEvents();

        auto result = ShuffleEngine::execute(group, room, rule, nameField);

        switch (result.status) {
            case ShuffleEngine::ResultStatus::Success:
                m_lastResult = result.record;
                m_lastRoom = room;
                m_hasResult = true;
                m_resultGrid->setData(room, result.record);
                m_statusLabel->setText(QString("抽取成功！已分配 %1 人").arg(result.record.assignments.size()));
                m_statusLabel->setStyleSheet(
                        "color: #52c41a; font-size: 13px; font-weight: bold; background: transparent;");
                break;

            case ShuffleEngine::ResultStatus::TooManyPersons:
                QMessageBox::warning(this, "人数超出", result.errorMessage);
                m_statusLabel->setText("抽取失败: 人数超出座位数");
                m_statusLabel->setStyleSheet("color: #ff4d4f; font-size: 13px; background: transparent;");
                break;

            case ShuffleEngine::ResultStatus::NoSolution:
                QMessageBox::warning(this, "无解", result.errorMessage);
                m_statusLabel->setText("抽取失败: 约束无法满足");
                m_statusLabel->setStyleSheet("color: #ff4d4f; font-size: 13px; background: transparent;");
                break;

            case ShuffleEngine::ResultStatus::InvalidInput:
                QMessageBox::warning(this, "输入错误", result.errorMessage);
                m_statusLabel->setText("抽取失败: 输入无效");
                m_statusLabel->setStyleSheet("color: #ff4d4f; font-size: 13px; background: transparent;");
                break;
        }
    }

    void ShufflePage::onSaveRecord() {
        if (!m_hasResult) {
            QMessageBox::information(this, "提示", "请先执行抽取");
            return;
        }

        DataManager::instance().saveRecord(m_lastResult);
        QMessageBox::information(this, "保存成功", QString("抽取记录已保存 (ID: %1)").arg(m_lastResult.id.left(8)));
    }

    void ShufflePage::onViewHistory() const {
        auto *historyPage = new HistoryPage(m_mainWindow);
        m_mainWindow->pushPage(historyPage);
    }

    // ============================================================
    // 导出为图片 (PNG)
    // ============================================================

    void ShufflePage::onExportImage() {
        if (!m_hasResult || !m_resultGrid) {
            QMessageBox::information(this, "提示", "请先执行抽取");
            return;
        }

        QPixmap pixmap = m_resultGrid->renderToPixmap();
        if (pixmap.isNull()) {
            QMessageBox::warning(this, "错误", "渲染图片失败");
            return;
        }

        QString defaultName = QString("座位分配_%1.png").arg(m_lastResult.timestamp.replace(':', '-'));

        QString filePath =
                QFileDialog::getSaveFileName(this, "导出图片", defaultName, "PNG 图片 (*.png);;所有文件 (*)");
        if (filePath.isEmpty())
            return;

        if (!filePath.endsWith(".png", Qt::CaseInsensitive))
            filePath += ".png";

        if (pixmap.save(filePath, "PNG")) {
            QMessageBox::information(this, "导出成功", QString("图片已保存到:\n%1").arg(filePath));
        } else {
            QMessageBox::critical(this, "导出失败", "图片保存失败，请检查路径是否可写。");
        }
    }

    // ============================================================
    // 导出为表格 (CSV)
    // ============================================================

    void ShufflePage::onExportCsv() {
        if (!m_hasResult || m_lastResult.assignments.isEmpty()) {
            QMessageBox::information(this, "提示", "请先执行抽取");
            return;
        }

        QString defaultName = QString("座位分配_%1.csv").arg(m_lastResult.timestamp.replace(':', '-'));

        QString filePath =
                QFileDialog::getSaveFileName(this, "导出表格", defaultName, "CSV 文件 (*.csv);;所有文件 (*)");
        if (filePath.isEmpty())
            return;

        if (!filePath.endsWith(".csv", Qt::CaseInsensitive))
            filePath += ".csv";

        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this, "导出失败", "无法写入文件:\n" + filePath);
            return;
        }

        QTextStream stream(&file);
        stream << QChar(0xFEFF); // UTF-8 BOM

        stream << "# 时间," << m_lastResult.timestamp << "\n";
        stream << "# 人员组," << m_lastResult.groupName << "\n";
        stream << "# 教室," << m_lastResult.classroomName << "\n";
        stream << "# 规则," << m_lastResult.ruleName << "\n";

        stream << "行号,列号,人员ID,姓名\n";

        for (const auto &a: m_lastResult.assignments) {
            auto csvEscape = [](const QString &s) -> QString {
                if (s.contains(',') || s.contains('"') || s.contains('\n')) {
                    QString escaped = s;
                    escaped.replace('"', "\"\"");
                    return '"' + escaped + '"';
                }
                return s;
            };

            stream << (a.row + 1) << "," << (a.col + 1) << "," << csvEscape(a.personId) << ","
                   << csvEscape(a.personName) << "\n";
        }

        file.close();
        QMessageBox::information(
                this, "导出成功",
                QString("CSV 文件已保存到:\n%1\n共 %2 条记录").arg(filePath).arg(m_lastResult.assignments.size()));
    }

} // namespace seat
