/**
 * @file MainWindow.h
 * @brief 主窗口
 *
 * 采用中央 QStackedWidget 进行页面切换，
 * 顶部导航栏对应四大功能模块：
 *   - 编座位 (座位抽取)
 *   - 人员管理
 *   - 教室管理
 *   - 规则管理
 *
 * 每个模块内部的 列表页 -> 详情页 跳转
 * 通过向 StackedWidget 推入/弹出页面实现。
 */

#ifndef SEATSHUFFLER_MAINWINDOW_H
#define SEATSHUFFLER_MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

namespace seat {
    class PersonGroupListPage;
    class ClassroomListPage;
    class RuleListPage;
    class ShufflePage;

    class MainWindow : public QMainWindow {
        Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = nullptr);
        ~MainWindow() override = default;

        /** 向页面栈中推入一个临时页面 (用于列表页 -> 详情页跳转) */
        void pushPage(QWidget *page);

        /** 弹出当前临时页面，回到上一级 */
        void popPage();

    private slots:
        void showHomePage();
        void showPersonGroupPage();
        void showClassroomPage();
        void showRulePage();
        void showShufflePage();

    private:
        void setupUi();
        void setupNavBar();
        void highlightNavButton(QPushButton *btn);

        QStackedWidget *m_stack = nullptr;

        // ─── 主导航按钮 ────────────────────────
        QPushButton *m_btnShuffle   = nullptr;
        QPushButton *m_btnPerson    = nullptr;
        QPushButton *m_btnRule      = nullptr;
        QPushButton *m_btnClassroom = nullptr;
        QVector<QPushButton *> m_navButtons;

        // ─── 核心页面 ──────────────────────────
        QWidget             *m_homePage      = nullptr;
        PersonGroupListPage *m_personPage    = nullptr;
        ClassroomListPage   *m_classroomPage = nullptr;
        RuleListPage        *m_rulePage      = nullptr;
        ShufflePage         *m_shufflePage   = nullptr;

        // ─── 页面栈管理 ────────────────────────
        QVector<QWidget *> m_tempPages;       ///< 临时推入的页面列表
        QWidget            *m_currentBasePage = nullptr; ///< 当前导航基础页面
    };
}

#endif // SEATSHUFFLER_MAINWINDOW_H