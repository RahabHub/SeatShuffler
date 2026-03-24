/**
 * @file HistoryPage.h
 * @brief 历史记录页面
 *
 * 展示所有抽取历史记录，支持查看详情和删除。
 */

#ifndef SEATSHUFFLER_HISTORYPAGE_H
#define SEATSHUFFLER_HISTORYPAGE_H

#include <QVBoxLayout>
#include <QWidget>

namespace seat {

    class MainWindow;

    class HistoryPage final : public QWidget {
        Q_OBJECT

    public:
        explicit HistoryPage(MainWindow *mainWindow, QWidget *parent = nullptr);
        ~HistoryPage() override = default;

    private slots:
        void onBack();
        void onDeleteRecord(const QString &id);
        void onViewRecord(const QString &id) const;

    private:
        void rebuildList();

        MainWindow *m_mainWindow;
        QVBoxLayout *m_listLayout = nullptr;
    };

} // namespace seat

#endif // SEATSHUFFLER_HISTORYPAGE_H
