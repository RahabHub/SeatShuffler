/**
 * @file PersonGroupDetailPage.h
 * @brief 人员组详情/编辑页面
 *
 * 功能:
 *   - 查看和编辑组名、备注
 *   - 管理自定义列 (添加/删除列)
 *   - 以表格形式展示和编辑人员数据
 *   - 手动添加/删除人员
 *   - 从 CSV/TSV 文件导入人员
 *   - 保存后清除脏标记，返回时不再重复询问
 */

#ifndef SEATSHUFFLER_PERSONGROUPDETAILPAGE_H
#define SEATSHUFFLER_PERSONGROUPDETAILPAGE_H

#include <QLineEdit>
#include <QTableWidget>
#include <QTextEdit>
#include <QWidget>
#include "src/models/PersonGroup.h"

namespace seat {

    class MainWindow;

    class PersonGroupDetailPage final : public QWidget {
        Q_OBJECT

    public:
        /**
         * @param groupId    要编辑的人员组 ID
         * @param mainWindow 主窗口指针，用于页面导航
         */
        explicit PersonGroupDetailPage(const QString &groupId, MainWindow *mainWindow, QWidget *parent = nullptr);
        ~PersonGroupDetailPage() override = default;

    private slots:
        void onBack();
        void onSave();
        void onAddColumn();
        void onRemoveColumn();
        void onAddPerson();
        void onRemovePerson();
        void onImportCsv();
        void onCellChanged(int row, int col);

    private:
        void loadData();
        void rebuildTable();
        void syncTableToModel();

        /** 标记数据已被修改 */
        void markDirty();

        MainWindow *m_mainWindow;
        QString m_groupId;
        PersonGroup m_group;

        QLineEdit *m_nameEdit = nullptr;
        QTextEdit *m_noteEdit = nullptr;
        QTableWidget *m_table = nullptr;

        bool m_blockCellSignal = false; ///< 防止 rebuildTable 时触发 cellChanged
        bool m_dirty = false; ///< 是否有未保存的修改
    };

} // namespace seat

#endif // SEATSHUFFLER_PERSONGROUPDETAILPAGE_H
