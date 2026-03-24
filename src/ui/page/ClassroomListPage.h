/**
 * @file ClassroomListPage.h
 * @brief 教室列表页面
 *
 * 以卡片形式展示所有教室，支持新建、删除、编辑。
 * 点击卡片进入 ClassroomEditorPage 编辑教室布局。
 */

#ifndef SEATSHUFFLER_CLASSROOMLISTPAGE_H
#define SEATSHUFFLER_CLASSROOMLISTPAGE_H

#include <QWidget>
#include <QVBoxLayout>

namespace seat {

    class MainWindow;

    class ClassroomListPage final : public QWidget {
        Q_OBJECT

    public:
        explicit ClassroomListPage(MainWindow *mainWindow, QWidget *parent = nullptr);
        ~ClassroomListPage() override = default;

        void refresh();

    private slots:
        void onCreateClassroom();
        void onDeleteClassroom(const QString &id);
        void onOpenClassroom(const QString &id) const;

    private:
        void rebuildCards();

        MainWindow  *m_mainWindow;
        QVBoxLayout *m_cardsLayout = nullptr;
        QWidget     *m_cardsContainer = nullptr;
    };

} // namespace seat

#endif // SEATSHUFFLER_CLASSROOMLISTPAGE_H
