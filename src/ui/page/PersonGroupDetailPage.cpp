/**
 * @file PersonGroupDetailPage.cpp
 * @brief 人员组详情页实现
 *
 * 改动:
 *   - 表格不再显示内部 UUID 列 (数据仍保留 id 字段)
 *   - 保存时对姓名、学号等常见字段检测重复值并提醒
 *   - 脏标记追踪
 */

#include "PersonGroupDetailPage.h"
#include "MainWindow.h"
#include "src/storage/DataManager.h"
#include "src/ui/common/AppStyle.h"

#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTextStream>
#include <QVBoxLayout>
#include <algorithm>

namespace seat {

    PersonGroupDetailPage::PersonGroupDetailPage(const QString &groupId, MainWindow *mainWindow, QWidget *parent) :
        QWidget(parent), m_mainWindow(mainWindow), m_groupId(groupId) {
        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(20, 16, 20, 16);
        layout->setSpacing(12);

        // ─── 顶部: 返回 + 保存 ──────────────────
        auto *topBar = new QHBoxLayout();
        auto *btnBack = new QPushButton("← 返回");
        btnBack->setCursor(Qt::PointingHandCursor);
        btnBack->setFocusPolicy(Qt::NoFocus);
        connect(btnBack, &QPushButton::clicked, this, &PersonGroupDetailPage::onBack);
        topBar->addWidget(btnBack);
        topBar->addStretch();
        auto *btnSave = new QPushButton("保存");
        AppStyle::applyPrimaryStyle(btnSave);
        btnSave->setMinimumWidth(80);
        btnSave->setCursor(Qt::PointingHandCursor);
        connect(btnSave, &QPushButton::clicked, this, &PersonGroupDetailPage::onSave);
        topBar->addWidget(btnSave);
        layout->addLayout(topBar);

        // ─── 组名和备注 ─────────────────────────
        auto *metaLayout = new QHBoxLayout();
        metaLayout->addWidget(new QLabel("组名:"));
        m_nameEdit = new QLineEdit();
        m_nameEdit->setPlaceholderText("请输入组名");
        metaLayout->addWidget(m_nameEdit, 1);
        metaLayout->addSpacing(16);
        metaLayout->addWidget(new QLabel("备注:"));
        m_noteEdit = new QTextEdit();
        m_noteEdit->setMaximumHeight(50);
        m_noteEdit->setPlaceholderText("可选备注信息");
        metaLayout->addWidget(m_noteEdit, 2);
        layout->addLayout(metaLayout);

        connect(m_nameEdit, &QLineEdit::textChanged, this, &PersonGroupDetailPage::markDirty);
        connect(m_noteEdit, &QTextEdit::textChanged, this, &PersonGroupDetailPage::markDirty);

        // ─── 列管理 + 数据操作按钮 ──────────────
        auto *toolBar = new QHBoxLayout();

        auto *btnAddCol = new QPushButton("添加列");
        btnAddCol->setCursor(Qt::PointingHandCursor);
        connect(btnAddCol, &QPushButton::clicked, this, &PersonGroupDetailPage::onAddColumn);
        toolBar->addWidget(btnAddCol);

        auto *btnRemCol = new QPushButton("删除列");
        btnRemCol->setCursor(Qt::PointingHandCursor);
        AppStyle::applyDangerStyle(btnRemCol);
        connect(btnRemCol, &QPushButton::clicked, this, &PersonGroupDetailPage::onRemoveColumn);
        toolBar->addWidget(btnRemCol);

        toolBar->addSpacing(20);

        auto *btnAddPerson = new QPushButton("添加人员");
        btnAddPerson->setCursor(Qt::PointingHandCursor);
        connect(btnAddPerson, &QPushButton::clicked, this, &PersonGroupDetailPage::onAddPerson);
        toolBar->addWidget(btnAddPerson);

        auto *btnRemPerson = new QPushButton("删除选中人员");
        btnRemPerson->setCursor(Qt::PointingHandCursor);
        AppStyle::applyDangerStyle(btnRemPerson);
        connect(btnRemPerson, &QPushButton::clicked, this, &PersonGroupDetailPage::onRemovePerson);
        toolBar->addWidget(btnRemPerson);

        toolBar->addSpacing(20);

        auto *btnImport = new QPushButton("导入 CSV/TSV");
        btnImport->setCursor(Qt::PointingHandCursor);
        connect(btnImport, &QPushButton::clicked, this, &PersonGroupDetailPage::onImportCsv);
        toolBar->addWidget(btnImport);

        toolBar->addStretch();
        layout->addLayout(toolBar);

        // ─── 数据表格 (不显示 ID 列) ───────────
        m_table = new QTableWidget();
        m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_table->setSelectionMode(QAbstractItemView::ExtendedSelection);
        m_table->horizontalHeader()->setStretchLastSection(true);
        m_table->setAlternatingRowColors(true);
        m_table->setStyleSheet("QTableWidget { alternate-background-color: #fafafa; }");
        connect(m_table, &QTableWidget::cellChanged, this, &PersonGroupDetailPage::onCellChanged);
        layout->addWidget(m_table, 1);

        loadData();
        m_dirty = false;
    }

    // ============================================================
    // 数据加载 & 表格重建
    // ============================================================

    void PersonGroupDetailPage::loadData() {
        m_group = DataManager::instance().loadGroup(m_groupId);
        m_nameEdit->setText(m_group.name);
        m_noteEdit->setPlainText(m_group.note);
        rebuildTable();
    }

    void PersonGroupDetailPage::rebuildTable() {
        m_blockCellSignal = true;

        // 不再显示 ID 列，仅显示自定义列
        const int colCount = m_group.columns.size();
        const int rowCount = m_group.persons.size();

        m_table->clear();
        m_table->setColumnCount(colCount);
        m_table->setRowCount(rowCount);

        // 表头: 仅自定义列
        QStringList headers;
        for (const auto &col: m_group.columns)
            headers << col.name;
        m_table->setHorizontalHeaderLabels(headers);

        // 填充数据
        for (int r = 0; r < rowCount; ++r) {
            const auto &person = m_group.persons[r];
            for (int c = 0; c < m_group.columns.size(); ++c) {
                const QString &colName = m_group.columns[c].name;
                auto *item = new QTableWidgetItem(person.fields.value(colName, ""));
                m_table->setItem(r, c, item);
            }
        }

        m_table->resizeColumnsToContents();
        m_blockCellSignal = false;
    }

    void PersonGroupDetailPage::syncTableToModel() {
        for (int r = 0; r < m_table->rowCount() && r < m_group.persons.size(); ++r) {
            for (int c = 0; c < m_group.columns.size(); ++c) {
                auto *item = m_table->item(r, c);
                if (item)
                    m_group.persons[r].fields[m_group.columns[c].name] = item->text();
            }
        }
        m_group.personCount = m_group.persons.size();
    }

    void PersonGroupDetailPage::markDirty() { m_dirty = true; }

    // ============================================================
    // 重复值检测 (软性提醒)
    // ============================================================

    /**
     * @brief 检测常见唯一性字段是否存在重复值
     *
     * 检查列名包含 "姓名"、"name"、"学号"、"编号" 的列，
     * 如存在重复值则弹出提醒 (不阻止保存)。
     */
    static void checkDuplicateFields(const PersonGroup &group, QWidget *parent) {
        // 需要检查的关键词
        static const QStringList keywords = {"姓名", "name", "学号", "编号", "工号"};

        for (const auto &col: group.columns) {
            bool shouldCheck = false;
            for (const auto &kw: keywords) {
                if (col.name.contains(kw, Qt::CaseInsensitive)) {
                    shouldCheck = true;
                    break;
                }
            }
            if (!shouldCheck)
                continue;

            // 统计每个值出现次数
            QMap<QString, int> valueCount;
            for (const auto &person: group.persons) {
                QString val = person.fields.value(col.name).trimmed();
                if (!val.isEmpty())
                    valueCount[val]++;
            }

            // 收集重复项
            QStringList duplicates;
            for (auto it = valueCount.begin(); it != valueCount.end(); ++it) {
                if (it.value() > 1)
                    duplicates << QString("%1 (×%2)").arg(it.key()).arg(it.value());
            }

            if (!duplicates.isEmpty()) {
                QMessageBox::warning(parent, "重复值提醒",
                                     QString("列「%1」中发现重复值:\n%2\n\n"
                                             "重复的值可能导致抽取结果中难以区分人员，建议检查确认。")
                                             .arg(col.name, duplicates.join("、")));
            }
        }
    }

    // ============================================================
    // 槽函数
    // ============================================================

    void PersonGroupDetailPage::onBack() {
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

    void PersonGroupDetailPage::onSave() {
        m_group.name = m_nameEdit->text().trimmed();
        m_group.note = m_noteEdit->toPlainText().trimmed();
        syncTableToModel();

        if (m_group.name.isEmpty()) {
            QMessageBox::warning(this, "提示", "组名不能为空");
            return;
        }

        // 保存前检测重复值 (软性提醒)
        checkDuplicateFields(m_group, this);

        DataManager::instance().saveGroup(m_group);
        m_dirty = false;
        QMessageBox::information(this, "保存成功", "人员组数据已保存");
    }

    void PersonGroupDetailPage::onAddColumn() {
        bool ok = false;
        QString colName = QInputDialog::getText(this, "添加列", "请输入列名:", QLineEdit::Normal, "", &ok);
        if (!ok || colName.trimmed().isEmpty())
            return;
        colName = colName.trimmed();

        for (const auto &col: m_group.columns) {
            if (col.name == colName) {
                QMessageBox::warning(this, "重复", "已存在同名列: " + colName);
                return;
            }
        }

        syncTableToModel();
        m_group.columns.append({colName, "string"});
        rebuildTable();
        markDirty();
    }

    void PersonGroupDetailPage::onRemoveColumn() {
        if (m_group.columns.isEmpty()) {
            QMessageBox::information(this, "提示", "没有可删除的列");
            return;
        }

        QStringList colNames;
        for (const auto &col: m_group.columns)
            colNames << col.name;

        bool ok = false;
        QString selected = QInputDialog::getItem(this, "删除列", "选择要删除的列:", colNames, 0, false, &ok);
        if (!ok)
            return;

        auto ret = QMessageBox::question(this, "确认", "删除列「" + selected + "」将丢失该列所有数据，确定吗？");
        if (ret != QMessageBox::Yes)
            return;

        syncTableToModel();
        m_group.columns.erase(std::remove_if(m_group.columns.begin(), m_group.columns.end(),
                                             [&](const ColumnDef &c) { return c.name == selected; }),
                              m_group.columns.end());
        for (auto &person: m_group.persons)
            person.fields.remove(selected);

        rebuildTable();
        markDirty();
    }

    void PersonGroupDetailPage::onAddPerson() {
        syncTableToModel();
        Person p;
        p.id = generateId();
        for (const auto &col: m_group.columns)
            p.fields[col.name] = "";
        m_group.persons.append(p);
        rebuildTable();
        markDirty();
        m_table->scrollToBottom();
    }

    void PersonGroupDetailPage::onRemovePerson() {
        auto selected = m_table->selectionModel()->selectedRows();
        if (selected.isEmpty()) {
            QMessageBox::information(this, "提示", "请先选择要删除的行");
            return;
        }

        auto ret =
                QMessageBox::question(this, "确认删除", QString("确定要删除选中的 %1 个人员吗？").arg(selected.size()));
        if (ret != QMessageBox::Yes)
            return;

        syncTableToModel();
        QVector<int> rows;
        for (const auto &idx: selected)
            rows.append(idx.row());
        std::sort(rows.begin(), rows.end(), std::greater<int>());
        for (int r: rows) {
            if (r >= 0 && r < m_group.persons.size())
                m_group.persons.removeAt(r);
        }
        rebuildTable();
        markDirty();
    }

    void PersonGroupDetailPage::onImportCsv() {
        QString filePath =
                QFileDialog::getOpenFileName(this, "选择文件", "", "表格文件 (*.csv *.tsv *.txt);;所有文件 (*)");
        if (filePath.isEmpty())
            return;

        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(this, "错误", "无法打开文件: " + filePath);
            return;
        }

        QTextStream stream(&file);
        QString firstLine = stream.readLine();
        if (firstLine.isEmpty()) {
            QMessageBox::warning(this, "提示", "文件为空");
            return;
        }

        QChar delimiter = ',';
        if (filePath.endsWith(".tsv", Qt::CaseInsensitive) || firstLine.contains('\t'))
            delimiter = '\t';

        QStringList headers = firstLine.split(delimiter);
        for (auto &h: headers) {
            h = h.trimmed();
            if (h.startsWith('"') && h.endsWith('"'))
                h = h.mid(1, h.size() - 2);
            if (h.startsWith(QChar(0xFEFF)))
                h = h.mid(1);
        }

        auto ret = QMessageBox::question(this, "导入选项",
                                         "是否用导入的列覆盖现有列定义？\n"
                                         "「Yes」= 覆盖列和数据\n「No」= 仅追加数据 (列名必须匹配)",
                                         QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (ret == QMessageBox::Cancel)
            return;

        syncTableToModel();

        if (ret == QMessageBox::Yes) {
            m_group.columns.clear();
            m_group.persons.clear();
            for (const auto &h: headers) {
                if (!h.isEmpty())
                    m_group.columns.append({h, "string"});
            }
        }

        int imported = 0;
        while (!stream.atEnd()) {
            QString line = stream.readLine().trimmed();
            if (line.isEmpty())
                continue;

            QStringList values = line.split(delimiter);
            Person p;
            p.id = generateId();

            if (ret == QMessageBox::Yes) {
                for (int i = 0; i < headers.size() && i < values.size(); ++i) {
                    QString val = values[i].trimmed();
                    if (val.startsWith('"') && val.endsWith('"'))
                        val = val.mid(1, val.size() - 2);
                    if (!headers[i].isEmpty())
                        p.fields[headers[i]] = val;
                }
            } else {
                for (int i = 0; i < headers.size() && i < values.size(); ++i) {
                    for (const auto &col: m_group.columns) {
                        if (col.name == headers[i]) {
                            QString val = values[i].trimmed();
                            if (val.startsWith('"') && val.endsWith('"'))
                                val = val.mid(1, val.size() - 2);
                            p.fields[col.name] = val;
                            break;
                        }
                    }
                }
            }

            m_group.persons.append(p);
            ++imported;
        }

        rebuildTable();
        markDirty();
        QMessageBox::information(this, "导入完成", QString("成功导入 %1 条人员记录").arg(imported));
    }

    void PersonGroupDetailPage::onCellChanged(int row, int col) {
        if (m_blockCellSignal)
            return;

        // 直接映射到自定义列 (无 ID 偏移)
        if (row >= 0 && row < m_group.persons.size() && col >= 0 && col < m_group.columns.size()) {
            auto *item = m_table->item(row, col);
            if (item)
                m_group.persons[row].fields[m_group.columns[col].name] = item->text();
        }
        markDirty();
    }

} // namespace seat
