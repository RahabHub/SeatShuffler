/**
 * @file RuleListPage.h
 * @brief 规则列表页面
 */

#ifndef SEATSHUFFLER_RULELISTPAGE_H
#define SEATSHUFFLER_RULELISTPAGE_H

#include <QVBoxLayout>
#include <QWidget>

namespace seat {

    class MainWindow;

    class RuleListPage final : public QWidget {
        Q_OBJECT

    public:
        explicit RuleListPage(MainWindow *mainWindow, QWidget *parent = nullptr);
        ~RuleListPage() override = default;

        void refresh();

    private slots:
        void onCreateRule();
        void onDeleteRule(const QString &id);
        void onOpenRule(const QString &id) const;

    private:
        void rebuildCards();

        MainWindow *m_mainWindow;
        QVBoxLayout *m_cardsLayout = nullptr;
        QWidget *m_cardsContainer = nullptr;
    };

} // namespace seat

#endif // SEATSHUFFLER_RULELISTPAGE_H
