/**
 * @file RuleEditorPage.cpp
 * @brief 可视化规则编辑页面实现
 *
 * 全新的规则编辑器:
 *   - 左侧: 教室网格，点击多选座位
 *   - 右侧: 约束工具栏 + 分组约束列表
 *   - 支持批量座位约束、全局约束、人员对约束
 */

#include "RuleEditorPage.h"
#include "MainWindow.h"
#include "src/storage/DataManager.h"
#include "src/ui/common/AppStyle.h"
#include "src/ui/widgets/SeatRuleGridWidget.h"

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSplitter>

namespace seat {

    RuleEditorPage::RuleEditorPage(const QString &ruleId, MainWindow *mainWindow, QWidget *parent) :
        QWidget(parent), m_mainWindow(mainWindow), m_ruleId(ruleId) {
        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(20, 16, 20, 16);
        layout->setSpacing(10);

        // ─── 顶部: 返回 + 保存 ─────────────────────
        auto *topBar = new QHBoxLayout();
        auto *btnBack = new QPushButton("← 返回");
        btnBack->setCursor(Qt::PointingHandCursor);
        btnBack->setFocusPolicy(Qt::NoFocus);
        connect(btnBack, &QPushButton::clicked, this, &RuleEditorPage::onBack);
        topBar->addWidget(btnBack);
        topBar->addStretch();
        auto *btnSave = new QPushButton("保存");
        AppStyle::applyPrimaryStyle(btnSave);
        btnSave->setMinimumWidth(80);
        btnSave->setCursor(Qt::PointingHandCursor);
        connect(btnSave, &QPushButton::clicked, this, &RuleEditorPage::onSave);
        topBar->addWidget(btnSave);
        layout->addLayout(topBar);

        // ─── 名称 & 备注 ───────────────────────────
        auto *metaRow = new QHBoxLayout();
        metaRow->addWidget(new QLabel("规则名称:"));
        m_nameEdit = new QLineEdit();
        m_nameEdit->setPlaceholderText("请输入规则名称");
        metaRow->addWidget(m_nameEdit, 1);
        metaRow->addSpacing(12);
        metaRow->addWidget(new QLabel("备注:"));
        m_noteEdit = new QTextEdit();
        m_noteEdit->setMaximumHeight(42);
        m_noteEdit->setPlaceholderText("可选");
        metaRow->addWidget(m_noteEdit, 2);
        layout->addLayout(metaRow);

        connect(m_nameEdit, &QLineEdit::textChanged, this, &RuleEditorPage::markDirty);
        connect(m_noteEdit, &QTextEdit::textChanged, this, &RuleEditorPage::markDirty);

        // ─── 关联教室 & 参考人员组 ─────────────────
        auto *refRow = new QHBoxLayout();
        refRow->addWidget(new QLabel("关联教室:"));
        m_classroomCombo = new QComboBox();
        m_classroomCombo->setMinimumWidth(180);
        m_classroomCombo->addItem("(不关联教室 - 纯文本编辑)", "");
        auto rooms = DataManager::instance().loadAllClassrooms();
        for (const auto &r: rooms)
            m_classroomCombo->addItem(QString("%1 (%2座)").arg(r.name).arg(r.seatCount()), r.id);
        connect(m_classroomCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                &RuleEditorPage::onClassroomChanged);
        refRow->addWidget(m_classroomCombo);

        refRow->addSpacing(16);
        refRow->addWidget(new QLabel("参考人员组:"));
        m_groupCombo = new QComboBox();
        m_groupCombo->setMinimumWidth(180);
        m_groupCombo->addItem("(无 - 仅需人员对约束时选择)", "");
        auto groups = DataManager::instance().loadAllGroups();
        for (const auto &g: groups)
            m_groupCombo->addItem(QString("%1 (%2人)").arg(g.name).arg(g.persons.size()), g.id);
        refRow->addWidget(m_groupCombo);
        refRow->addStretch();
        layout->addLayout(refRow);

        // ─── 主内容区: 左=网格 右=工具+约束列表 ────
        auto *splitter = new QSplitter(Qt::Horizontal);

        // 左面板: 网格
        auto *leftWidget = new QWidget();
        auto *leftLayout = new QVBoxLayout(leftWidget);
        leftLayout->setContentsMargins(0, 0, 0, 0);

        auto *gridScroll = new QScrollArea();
        gridScroll->setWidgetResizable(true);
        m_gridWidget = new SeatRuleGridWidget();
        gridScroll->setWidget(m_gridWidget);
        leftLayout->addWidget(gridScroll, 1);

        m_selectionLabel = new QLabel("请先选择关联教室");
        m_selectionLabel->setStyleSheet("color: #8c8c8c; font-size: 12px; background: transparent; padding: 4px;");
        leftLayout->addWidget(m_selectionLabel);

        connect(m_gridWidget, &SeatRuleGridWidget::selectionChanged, this, &RuleEditorPage::onSelectionChanged);

        splitter->addWidget(leftWidget);

        // 右面板: 工具栏 + 约束列表
        auto *rightWidget = new QWidget();
        rightWidget->setMinimumWidth(300);
        auto *rightLayout = new QVBoxLayout(rightWidget);
        rightLayout->setContentsMargins(8, 0, 0, 0);
        rightLayout->setSpacing(8);

        // 座位约束工具
        auto *seatToolBox = new QGroupBox("座位约束 (先在左侧选中座位)");
        auto *seatToolLayout = new QVBoxLayout(seatToolBox);
        auto *btnSetSeat = new QPushButton("为选中座位设置字段约束");
        btnSetSeat->setCursor(Qt::PointingHandCursor);
        connect(btnSetSeat, &QPushButton::clicked, this, &RuleEditorPage::onSetSeatConstraint);
        seatToolLayout->addWidget(btnSetSeat);
        auto *btnClearSeat = new QPushButton("清除选中座位的约束");
        btnClearSeat->setCursor(Qt::PointingHandCursor);
        connect(btnClearSeat, &QPushButton::clicked, this, &RuleEditorPage::onClearSeatConstraint);
        seatToolLayout->addWidget(btnClearSeat);
        rightLayout->addWidget(seatToolBox);

        // 全局约束工具
        auto *globalToolBox = new QGroupBox("全局约束");
        auto *globalToolLayout = new QVBoxLayout(globalToolBox);
        auto *btnGlobal = new QPushButton("添加相邻约束 (不同/相同)");
        btnGlobal->setCursor(Qt::PointingHandCursor);
        connect(btnGlobal, &QPushButton::clicked, this, &RuleEditorPage::onAddGlobalConstraint);
        globalToolLayout->addWidget(btnGlobal);
        rightLayout->addWidget(globalToolBox);

        // 人员对约束工具
        auto *pairToolBox = new QGroupBox("人员对约束");
        auto *pairToolLayout = new QVBoxLayout(pairToolBox);
        auto *btnPair = new QPushButton("添加禁止同桌 / 强制同桌");
        btnPair->setCursor(Qt::PointingHandCursor);
        connect(btnPair, &QPushButton::clicked, this, &RuleEditorPage::onAddPairConstraint);
        pairToolLayout->addWidget(btnPair);
        auto *pairHint = new QLabel("需先选择「参考人员组」");
        pairHint->setStyleSheet("color: #8c8c8c; font-size: 11px; background: transparent;");
        pairToolLayout->addWidget(pairHint);
        rightLayout->addWidget(pairToolBox);

        // 约束列表
        auto *listLabel = new QLabel("当前约束列表:");
        listLabel->setStyleSheet("font-weight: bold; background: transparent;");
        rightLayout->addWidget(listLabel);

        auto *constraintScroll = new QScrollArea();
        constraintScroll->setWidgetResizable(true);
        constraintScroll->setFrameShape(QFrame::NoFrame);
        auto *constraintContainer = new QWidget();
        constraintContainer->setStyleSheet("background: transparent;");
        m_constraintListLayout = new QVBoxLayout(constraintContainer);
        m_constraintListLayout->setAlignment(Qt::AlignTop);
        m_constraintListLayout->setSpacing(6);
        constraintScroll->setWidget(constraintContainer);
        rightLayout->addWidget(constraintScroll, 1);

        splitter->addWidget(rightWidget);
        splitter->setStretchFactor(0, 3); // 左侧占更多空间
        splitter->setStretchFactor(1, 2);

        layout->addWidget(splitter, 1);

        // ─── 加载数据 ─────────────────────────────
        loadData();
        m_dirty = false;
    }

    // ============================================================
    // 数据加载
    // ============================================================

    void RuleEditorPage::loadData() {
        m_rule = DataManager::instance().loadRule(m_ruleId);
        m_nameEdit->setText(m_rule.name);
        m_noteEdit->setPlainText(m_rule.note);

        // 恢复关联教室
        if (!m_rule.classroomId.isEmpty()) {
            int idx = m_classroomCombo->findData(m_rule.classroomId);
            if (idx >= 0)
                m_classroomCombo->setCurrentIndex(idx);
        }

        // 恢复参考人员组
        if (!m_rule.referenceGroupId.isEmpty()) {
            int idx = m_groupCombo->findData(m_rule.referenceGroupId);
            if (idx >= 0)
                m_groupCombo->setCurrentIndex(idx);
        }

        refreshGrid();
        rebuildConstraintList();
    }

    void RuleEditorPage::refreshGrid() {
        // 构造期间 combo 信号可能先于控件初始化触发，需保护
        if (!m_gridWidget || !m_selectionLabel)
            return;

        QString classroomId = m_classroomCombo->currentData().toString();
        if (classroomId.isEmpty()) {
            m_gridWidget->setClassroom(Classroom{});
            m_selectionLabel->setText("请先选择关联教室以使用可视化编辑");
            return;
        }
        auto room = DataManager::instance().loadClassroom(classroomId);
        if (room.id.isEmpty())
            return;
        m_gridWidget->setClassroom(room);
        m_gridWidget->updateConstraintOverlay(m_rule.constraints);
        onSelectionChanged();
    }

    void RuleEditorPage::markDirty() { m_dirty = true; }

    PersonGroup RuleEditorPage::loadReferenceGroup() const {
        QString gid = m_groupCombo->currentData().toString();
        if (gid.isEmpty())
            return {};
        return DataManager::instance().loadGroup(gid);
    }

    // ============================================================
    // 约束分组 (用于 UI 显示)
    // ============================================================

    void RuleEditorPage::buildConstraintGroups() {
        m_groups.clear();

        // 按 (type, field, value) 分组 SeatFieldMatch
        QMap<QString, ConstraintGroup> seatGroups;
        for (int i = 0; i < m_rule.constraints.size(); ++i) {
            const auto &c = m_rule.constraints[i];
            if (c.type == ConstraintType::SeatFieldMatch) {
                QString key = QString("seat_%1_%2").arg(c.params["field"].toString(), c.params["value"].toString());
                if (!seatGroups.contains(key)) {
                    seatGroups[key].description =
                            QString("座位约束: %1 = %2")
                                    .arg(c.params["field"].toString(), c.params["value"].toString());
                }
                seatGroups[key].indices.append(i);
            }
        }
        // 为每个分组追加座位坐标信息
        for (auto it = seatGroups.begin(); it != seatGroups.end(); ++it) {
            QStringList coords;
            for (int idx: it.value().indices) {
                const auto &p = m_rule.constraints[idx].params;
                coords << QString("(%1,%2)").arg(p["row"].toInt() + 1).arg(p["col"].toInt() + 1);
            }
            it.value().description += "  →  " + coords.join(" ");
            m_groups.append(it.value());
        }

        // 其他约束逐条添加
        for (int i = 0; i < m_rule.constraints.size(); ++i) {
            const auto &c = m_rule.constraints[i];
            ConstraintGroup g;
            g.indices = {i};
            switch (c.type) {
                case ConstraintType::SeatFieldMatch:
                    continue; // 已在上面分组处理
                case ConstraintType::NoAdjacentSame:
                    g.description =
                            QString("全局: 相邻座位「%1」不能相同 (空值跳过)").arg(c.params["field"].toString());
                    break;
                case ConstraintType::ForceAdjacentSame:
                    g.description =
                            QString("全局: 相邻座位「%1」必须相同 (空值跳过)").arg(c.params["field"].toString());
                    break;
                case ConstraintType::ColumnFieldMatch:
                    g.description = QString("第%1列: %2 = %3")
                                            .arg(c.params["col"].toInt() + 1)
                                            .arg(c.params["field"].toString(), c.params["value"].toString());
                    break;
                case ConstraintType::ZoneFieldMatch:
                    g.description = QString("区域(%1,%2)-(%3,%4): %5 = %6")
                                            .arg(c.params["rowStart"].toInt() + 1)
                                            .arg(c.params["colStart"].toInt() + 1)
                                            .arg(c.params["rowEnd"].toInt() + 1)
                                            .arg(c.params["colEnd"].toInt() + 1)
                                            .arg(c.params["field"].toString(), c.params["value"].toString());
                    break;
                case ConstraintType::RowPattern: {
                    QStringList pat;
                    for (const auto &v: c.params["pattern"].toArray())
                        pat << v.toString();
                    g.description = QString("第%1行模式: %2 = %3")
                                            .arg(c.params["row"].toInt() + 1)
                                            .arg(c.params["field"].toString(), pat.join(" "));
                    break;
                }
                case ConstraintType::ForbidPair:
                    g.description =
                            QString("禁止同桌: %1 & %2")
                                    .arg(c.params["personName1"].toString(), c.params["personName2"].toString());
                    break;
                case ConstraintType::ForcePair:
                    g.description =
                            QString("强制同桌: %1 & %2")
                                    .arg(c.params["personName1"].toString(), c.params["personName2"].toString());
                    break;
            }
            m_groups.append(g);
        }
    }

    void RuleEditorPage::rebuildConstraintList() {
        QLayoutItem *child;
        while ((child = m_constraintListLayout->takeAt(0)) != nullptr) {
            if (child->widget())
                child->widget()->deleteLater();
            delete child;
        }

        buildConstraintGroups();

        if (m_groups.isEmpty()) {
            auto *emptyLabel = new QLabel("暂无约束。\n使用左侧网格或右侧工具添加。");
            emptyLabel->setAlignment(Qt::AlignCenter);
            emptyLabel->setStyleSheet("color: #8c8c8c; padding: 20px; background: transparent;");
            m_constraintListLayout->addWidget(emptyLabel);
            return;
        }

        for (int gi = 0; gi < m_groups.size(); ++gi) {
            auto *card = new QFrame();
            card->setFrameShape(QFrame::StyledPanel);
            auto *cardLayout = new QVBoxLayout(card);
            cardLayout->setContentsMargins(10, 8, 10, 8);
            cardLayout->setSpacing(6);

            auto *descLabel = new QLabel(m_groups[gi].description);
            descLabel->setWordWrap(true);
            descLabel->setStyleSheet("color: #1f1f1f; font-size: 12px; background: transparent;");
            cardLayout->addWidget(descLabel);

            // 删除按钮右对齐
            auto *btnRow = new QHBoxLayout();
            btnRow->addStretch();
            auto *btnDel = new QPushButton("删除");
            btnDel->setFixedSize(56, 28);
            btnDel->setCursor(Qt::PointingHandCursor);
            AppStyle::applyDangerStyle(btnDel);
            connect(btnDel, &QPushButton::clicked, this, [this, gi]() { onRemoveConstraintGroup(gi); });
            btnRow->addWidget(btnDel);
            cardLayout->addLayout(btnRow);

            m_constraintListLayout->addWidget(card);
        }
    }

    // ============================================================
    // 槽函数
    // ============================================================

    void RuleEditorPage::onBack() {
        if (m_dirty) {
            auto ret = QMessageBox::question(this, "返回", "有未保存的修改，是否保存？",
                                             QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
            if (ret == QMessageBox::Cancel)
                return;
            if (ret == QMessageBox::Yes)
                onSave();
        }
        m_mainWindow->popPage();
    }

    void RuleEditorPage::onSave() {
        m_rule.name = m_nameEdit->text().trimmed();
        m_rule.note = m_noteEdit->toPlainText().trimmed();
        m_rule.classroomId = m_classroomCombo->currentData().toString();
        m_rule.referenceGroupId = m_groupCombo->currentData().toString();

        if (m_rule.name.isEmpty()) {
            QMessageBox::warning(this, "提示", "规则名称不能为空");
            return;
        }

        DataManager::instance().saveRule(m_rule);
        m_dirty = false;
        QMessageBox::information(this, "保存成功", "规则已保存");
    }

    void RuleEditorPage::onClassroomChanged(int) {
        refreshGrid();
        markDirty();
    }

    void RuleEditorPage::onSelectionChanged() {
        if (!m_gridWidget || !m_selectionLabel)
            return;
        int count = m_gridWidget->selectedCount();
        if (count > 0)
            m_selectionLabel->setText(QString("已选中 %1 个座位 — 使用右侧工具设置约束").arg(count));
        else
            m_selectionLabel->setText("点击网格中的座位进行选中");
    }

    // ============================================================
    // 座位约束操作
    // ============================================================

    void RuleEditorPage::onSetSeatConstraint() {
        auto selected = m_gridWidget->selectedSeats();
        if (selected.isEmpty()) {
            QMessageBox::information(this, "提示", "请先在网格中选中座位");
            return;
        }

        // 弹出对话框让用户输入字段名和值
        QDialog dlg(this);
        dlg.setWindowTitle("设置座位约束");
        dlg.setMinimumWidth(350);
        auto *mainLayout = new QVBoxLayout(&dlg);

        auto *form = new QFormLayout();
        auto *fieldEdit = new QLineEdit();
        fieldEdit->setPlaceholderText("例如: 性别");

        // 如果有参考人员组，提供字段下拉
        auto *fieldCombo = new QComboBox();
        fieldCombo->setEditable(true);
        auto group = loadReferenceGroup();
        if (!group.id.isEmpty()) {
            for (const auto &col: group.columns)
                fieldCombo->addItem(col.name);
        }
        form->addRow("字段名:", fieldCombo);

        auto *valueEdit = new QLineEdit();
        valueEdit->setPlaceholderText("例如: 男");
        form->addRow("约束值:", valueEdit);

        auto *hint = new QLabel(QString("将为选中的 %1 个座位批量设置此约束").arg(selected.size()));
        hint->setStyleSheet("color: #8c8c8c; font-size: 11px;");
        form->addRow(hint);

        mainLayout->addLayout(form);
        auto *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        connect(btnBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
        connect(btnBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
        mainLayout->addWidget(btnBox);

        if (dlg.exec() != QDialog::Accepted)
            return;

        QString field = fieldCombo->currentText().trimmed();
        QString value = valueEdit->text().trimmed();
        if (field.isEmpty() || value.isEmpty()) {
            QMessageBox::warning(this, "提示", "字段名和约束值不能为空");
            return;
        }

        // 批量创建 SeatFieldMatch
        for (const auto &[row, col]: selected) {
            // 检查是否已有同位置同字段的约束，有则更新
            bool found = false;
            for (auto &c: m_rule.constraints) {
                if (c.type == ConstraintType::SeatFieldMatch && c.params["row"].toInt() == row &&
                    c.params["col"].toInt() == col && c.params["field"].toString() == field) {
                    c.params["value"] = value;
                    found = true;
                    break;
                }
            }
            if (!found) {
                Constraint c;
                c.type = ConstraintType::SeatFieldMatch;
                c.params = {{"row", row}, {"col", col}, {"field", field}, {"value", value}};
                m_rule.constraints.append(c);
            }
        }

        m_gridWidget->clearSelection();
        m_gridWidget->updateConstraintOverlay(m_rule.constraints);
        rebuildConstraintList();
        markDirty();
    }

    void RuleEditorPage::onClearSeatConstraint() {
        auto selected = m_gridWidget->selectedSeats();
        if (selected.isEmpty()) {
            QMessageBox::information(this, "提示", "请先在网格中选中座位");
            return;
        }

        QSet<QPair<int, int>> selSet(selected.begin(), selected.end());

        // 移除匹配的 SeatFieldMatch
        m_rule.constraints.erase(std::remove_if(m_rule.constraints.begin(), m_rule.constraints.end(),
                                                [&](const Constraint &c) {
                                                    if (c.type != ConstraintType::SeatFieldMatch)
                                                        return false;
                                                    auto key =
                                                            qMakePair(c.params["row"].toInt(), c.params["col"].toInt());
                                                    return selSet.contains(key);
                                                }),
                                 m_rule.constraints.end());

        m_gridWidget->clearSelection();
        m_gridWidget->updateConstraintOverlay(m_rule.constraints);
        rebuildConstraintList();
        markDirty();
    }

    // ============================================================
    // 全局约束
    // ============================================================

    void RuleEditorPage::onAddGlobalConstraint() {
        QStringList types = {"相邻不同约束 (同行相邻座位某字段不同)", "相邻相同约束 (同行相邻座位某字段相同)"};
        bool ok = false;
        QString selected = QInputDialog::getItem(this, "添加全局约束", "约束类型:", types, 0, false, &ok);
        if (!ok)
            return;

        int typeIdx = types.indexOf(selected);

        // 字段名输入
        auto *fieldCombo = new QComboBox();
        fieldCombo->setEditable(true);
        auto group = loadReferenceGroup();
        QStringList fieldNames;
        if (!group.id.isEmpty()) {
            for (const auto &col: group.columns)
                fieldNames << col.name;
        }

        QString field = QInputDialog::getText(this, "字段名", "输入字段名 (空值将自动跳过比较):", QLineEdit::Normal,
                                              fieldNames.isEmpty() ? "性别" : fieldNames.first(), &ok);
        if (!ok || field.trimmed().isEmpty())
            return;

        Constraint c;
        c.type = (typeIdx == 0) ? ConstraintType::NoAdjacentSame : ConstraintType::ForceAdjacentSame;
        c.params = {{"field", field.trimmed()}};
        m_rule.constraints.append(c);

        rebuildConstraintList();
        markDirty();
    }

    // ============================================================
    // 人员对约束
    // ============================================================

    void RuleEditorPage::onAddPairConstraint() {
        auto group = loadReferenceGroup();
        if (group.id.isEmpty() || group.persons.size() < 2) {
            QMessageBox::warning(this, "提示", "请先在上方选择一个包含至少2人的参考人员组");
            return;
        }

        // 约束类型
        QStringList types = {"禁止同桌 (两人不能坐相邻座位)", "强制同桌 (两人必须坐相邻座位)"};
        bool ok = false;
        QString selected = QInputDialog::getItem(this, "人员对约束", "约束类型:", types, 0, false, &ok);
        if (!ok)
            return;
        int typeIdx = types.indexOf(selected);

        // 确定显示用的姓名字段
        QString nameField;
        for (const auto &col: group.columns) {
            QString lower = col.name.toLower();
            if (lower.contains("姓名") || lower.contains("name") || lower == "名字") {
                nameField = col.name;
                break;
            }
        }
        if (nameField.isEmpty() && !group.columns.isEmpty())
            nameField = group.columns.first().name;

        // 构建人员显示列表
        QStringList personList;
        for (const auto &p: group.persons) {
            QString display = nameField.isEmpty() ? p.id : p.fields.value(nameField, p.id);
            personList << display;
        }

        // 选择第一个人
        QString person1 = QInputDialog::getItem(this, "选择人员 1", "第一个人:", personList, 0, false, &ok);
        if (!ok)
            return;
        int idx1 = personList.indexOf(person1);

        // 选择第二个人
        QString person2 = QInputDialog::getItem(this, "选择人员 2", "第二个人:", personList, 0, false, &ok);
        if (!ok)
            return;
        int idx2 = personList.indexOf(person2);

        if (idx1 == idx2) {
            QMessageBox::warning(this, "提示", "不能选择同一个人");
            return;
        }

        Constraint c;
        c.type = (typeIdx == 0) ? ConstraintType::ForbidPair : ConstraintType::ForcePair;
        c.params = {{"personId1", group.persons[idx1].id},
                    {"personName1", personList[idx1]},
                    {"personId2", group.persons[idx2].id},
                    {"personName2", personList[idx2]}};
        m_rule.constraints.append(c);

        rebuildConstraintList();
        markDirty();
    }

    // ============================================================
    // 删除约束
    // ============================================================

    void RuleEditorPage::onRemoveConstraintGroup(int groupIndex) {
        if (groupIndex < 0 || groupIndex >= m_groups.size())
            return;

        auto ret = QMessageBox::question(this, "确认删除", "确定要删除这组约束吗？");
        if (ret != QMessageBox::Yes)
            return;

        // 收集要删除的索引 (从大到小排序以避免偏移)
        QVector<int> toRemove = m_groups[groupIndex].indices;
        std::sort(toRemove.begin(), toRemove.end(), std::greater<int>());
        for (int idx: toRemove) {
            if (idx >= 0 && idx < m_rule.constraints.size())
                m_rule.constraints.removeAt(idx);
        }

        m_gridWidget->updateConstraintOverlay(m_rule.constraints);
        rebuildConstraintList();
        markDirty();
    }

} // namespace seat
