/**
 * @file PersonGroupListPage.h
 * @brief 人员组列表页面
 *
 * 以卡片形式展示所有人员组，支持新建和删除。
 * 点击卡片可跳转到 PersonGroupDetailPage 查看/编辑详情。
 */

#ifndef SEATSHUFFLER_PERSONGROUPLISTPAGE_H
#define SEATSHUFFLER_PERSONGROUPLISTPAGE_H

#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

namespace seat {

    class MainWindow;

    class PersonGroupListPage final : public QWidget {
        Q_OBJECT

    public:
        explicit PersonGroupListPage(MainWindow *mainWindow, QWidget *parent = nullptr);
        ~PersonGroupListPage() override = default;

        /** 刷新列表 */
        void refresh();

    private slots:
        void onCreateGroup();
        void onDeleteGroup(const QString &id);
        void onOpenGroup(const QString &id) const;

    private:
        void rebuildCards();

        MainWindow *m_mainWindow;
        QVBoxLayout *m_cardsLayout = nullptr; // 卡片容器的布局
        QWidget *m_cardsContainer = nullptr;
    };

} // namespace seat

#endif // SEATSHUFFLER_PERSONGROUPLISTPAGE_H
