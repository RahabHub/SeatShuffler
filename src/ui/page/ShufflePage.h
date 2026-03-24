/**
 * @file ShufflePage.h
 * @brief 正式抽取页面
 *
 * 用户选择人员组、教室、规则 (可选)、姓名字段，
 * 然后执行分配，结果以网格形式展示。
 * 可保存历史记录，可导出为图片 (PNG) 和表格 (CSV)。
 */

#ifndef SEATSHUFFLER_SHUFFLEPAGE_H
#define SEATSHUFFLER_SHUFFLEPAGE_H

#include <QComboBox>
#include <QLabel>
#include <QWidget>
#include "src/models/Classroom.h"
#include "src/models/ShuffleRecord.h"

namespace seat {

    class MainWindow;
    class ResultGridWidget;

    class ShufflePage : public QWidget {
        Q_OBJECT

    public:
        explicit ShufflePage(MainWindow *mainWindow, QWidget *parent = nullptr);
        ~ShufflePage() override = default;

        /** 刷新下拉框 (进入页面时调用) */
        void refresh();

    private slots:
        void onShuffle();
        void onSaveRecord();
        void onViewHistory() const;
        void onExportImage();
        void onExportCsv();

    private:
        void populateCombos() const;

        MainWindow *m_mainWindow;
        QComboBox *m_groupCombo = nullptr;
        QComboBox *m_classroomCombo = nullptr;
        QComboBox *m_ruleCombo = nullptr;
        QComboBox *m_nameFieldCombo = nullptr;
        QLabel *m_statusLabel = nullptr;
        ResultGridWidget *m_resultGrid = nullptr;

        ShuffleRecord m_lastResult; ///< 最近一次抽取结果
        Classroom m_lastRoom; ///< 对应的教室 (用于结果绘制)
        bool m_hasResult = false;
    };

} // namespace seat

#endif // SEATSHUFFLER_SHUFFLEPAGE_H
