/**
 * @file ClassroomEditorPage.h
 * @brief 教室布局编辑页面
 *
 * 提供一个网格化的教室编辑器:
 *   - 调整行列数
 *   - 点击/拖拽单元格切换类型 (座位/过道/空位)
 *   - 实时预览布局
 *   - 保存后自动清除脏标记，返回时不再重复询问
 */

#ifndef SEATSHUFFLER_CLASSROOMEDITORPAGE_H
#define SEATSHUFFLER_CLASSROOMEDITORPAGE_H

#include <QLineEdit>
#include <QSpinBox>
#include <QTextEdit>
#include <QWidget>
#include "src/models/Classroom.h"

namespace seat {

    class MainWindow;
    class ClassroomGridWidget;

    class ClassroomEditorPage final : public QWidget {
        Q_OBJECT

    public:
        explicit ClassroomEditorPage(QString classroomId, MainWindow *mainWindow, QWidget *parent = nullptr);
        ~ClassroomEditorPage() override = default;

    private slots:
        void onBack();
        void onSave();
        void onRowsChanged(int value);
        void onColsChanged(int value);

    private:
        void loadData();

        /** 标记数据已被修改 */
        void markDirty();

        MainWindow *m_mainWindow;
        QString m_classroomId;
        Classroom m_classroom;

        QLineEdit *m_nameEdit = nullptr;
        QTextEdit *m_noteEdit = nullptr;
        QSpinBox *m_rowsSpin = nullptr;
        QSpinBox *m_colsSpin = nullptr;
        ClassroomGridWidget *m_gridWidget = nullptr;

        bool m_dirty = false; ///< 是否有未保存的修改
    };

} // namespace seat

#endif // SEATSHUFFLER_CLASSROOMEDITORPAGE_H
