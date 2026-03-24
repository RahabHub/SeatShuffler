/**
 * @file RuleEditorPage.h
 * @brief 可视化规则编辑页面
 *
 * 全新的规则编辑体验:
 *   - 关联教室，在网格上点选座位批量设置约束
 *   - 支持全局约束 (相邻不同/相同)
 *   - 支持人员对约束 (禁止同桌/强制同桌)
 *   - 约束列表分组展示，支持批量删除
 */

#ifndef SEATSHUFFLER_RULEEDITORPAGE_H
#define SEATSHUFFLER_RULEEDITORPAGE_H

#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>
#include "src/models/PersonGroup.h"
#include "src/models/Rule.h"

namespace seat {

    class MainWindow;
    class SeatRuleGridWidget;

    class RuleEditorPage : public QWidget {
        Q_OBJECT

    public:
        explicit RuleEditorPage(const QString &ruleId, MainWindow *mainWindow, QWidget *parent = nullptr);
        ~RuleEditorPage() override = default;

    private slots:
        void onBack();
        void onSave();

        /** 关联教室变更 */
        void onClassroomChanged(int index);

        /** 座位约束工具 */
        void onSetSeatConstraint();
        void onClearSeatConstraint();

        /** 全局约束 */
        void onAddGlobalConstraint();

        /** 人员对约束 */
        void onAddPairConstraint();

        /** 删除约束 */
        void onRemoveConstraintGroup(int groupIndex);

        /** 选中状态变更时更新UI */
        void onSelectionChanged();

    private:
        void loadData();
        void refreshGrid();
        void rebuildConstraintList();

        /** 标记数据已被修改 */
        void markDirty();

        /** 获取参考人员组 (用于人员对选择) */
        PersonGroup loadReferenceGroup() const;

        MainWindow *m_mainWindow;
        QString m_ruleId;
        Rule m_rule;

        // ─── UI 控件 ──────────────────────────────
        QLineEdit *m_nameEdit = nullptr;
        QTextEdit *m_noteEdit = nullptr;
        QComboBox *m_classroomCombo = nullptr;
        QComboBox *m_groupCombo = nullptr;
        SeatRuleGridWidget *m_gridWidget = nullptr;
        QLabel *m_selectionLabel = nullptr;
        QVBoxLayout *m_constraintListLayout = nullptr;

        bool m_dirty = false;

        /**
         * @brief 约束分组描述，用于 UI 列表展示
         *
         * 将多个同类型约束合并为一组显示 (如同一批 SeatFieldMatch)
         */
        struct ConstraintGroup {
            QString description; ///< 可读描述
            QVector<int> indices; ///< 对应 m_rule.constraints 中的索引
        };
        QVector<ConstraintGroup> m_groups;

        /** 构建约束分组 */
        void buildConstraintGroups();
    };

} // namespace seat

#endif // SEATSHUFFLER_RULEEDITORPAGE_H
