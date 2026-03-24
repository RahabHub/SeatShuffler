/**
 * @file MainWindow.cpp
 * @brief 主窗口实现
 *
 * 负责:
 *   - 构建顶部导航栏与中央页面栈
 *   - 四大模块页面的切换
 *   - 临时页面的推入/弹出管理
 *   - 应用全局现代化样式和程序图标
 */

#include "MainWindow.h"
#include "ClassroomListPage.h"
#include "PersonGroupListPage.h"
#include "RuleListPage.h"
#include "ShufflePage.h"
#include "src/ui/common/AppStyle.h"

#include <QApplication>
#include <QFrame>
#include <QIcon>
#include <QLabel>

namespace seat {

    MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
        setWindowTitle("SeatShuffler - 座位抽取系统");
        setWindowIcon(QIcon(":/resources/logo.png"));
        resize(1100, 750);

        // 应用全局样式表
        qApp->setStyleSheet(AppStyle::styleSheet());

        setupUi();
    }

    // ============================================================
    // UI 构建
    // ============================================================

    void MainWindow::setupUi() {
        auto *centralWidget = new QWidget(this);
        centralWidget->setStyleSheet("background-color: #f0f2f5;");
        setCentralWidget(centralWidget);

        auto *mainLayout = new QVBoxLayout(centralWidget);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->setSpacing(0);

        // ─── 顶部导航栏 ────────────────────────────────
        setupNavBar();

        auto *navFrame = new QFrame();
        navFrame->setStyleSheet("QFrame {"
                                "  background-color: #ffffff;"
                                "  border: none;"
                                "  border-bottom: 1px solid #e8e8e8;"
                                "}");
        navFrame->setFixedHeight(52);

        auto *navLayout = new QHBoxLayout(navFrame);
        navLayout->setContentsMargins(20, 0, 20, 0);
        navLayout->setSpacing(6);

        // 导航栏左侧: Logo 图标 + 标题
        auto *logoLabel = new QLabel();
        QPixmap logoPix(":/resources/logo.png");
        if (!logoPix.isNull()) {
            logoLabel->setPixmap(logoPix.scaled(28, 28, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            logoLabel->setFixedSize(28, 28);
            logoLabel->setStyleSheet("background: transparent;");
            navLayout->addWidget(logoLabel);
            navLayout->addSpacing(6);
        }

        auto *titleLabel = new QLabel("SeatShuffler");
        titleLabel->setStyleSheet("font-size: 17px;"
                                  "font-weight: bold;"
                                  "color: #1677ff;"
                                  "background: transparent;");
        navLayout->addWidget(titleLabel);
        navLayout->addSpacing(24);

        for (auto *btn: m_navButtons) {
            navLayout->addWidget(btn);
        }
        navLayout->addStretch();

        mainLayout->addWidget(navFrame);

        // ─── 页面栈 ────────────────────────────────────
        m_stack = new QStackedWidget();
        m_stack->setStyleSheet("background-color: #f0f2f5;");
        mainLayout->addWidget(m_stack, 1);

        // 创建首页 (欢迎页面，带居中 Logo)
        m_homePage = new QWidget();
        m_homePage->setStyleSheet("background-color: #f0f2f5;");
        auto *homeLayout = new QVBoxLayout(m_homePage);
        homeLayout->addStretch();

        auto *homeLogoLabel = new QLabel();
        QPixmap homeLogo(":/resources/logo.png");
        if (!homeLogo.isNull()) {
            homeLogoLabel->setPixmap(homeLogo.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            homeLogoLabel->setAlignment(Qt::AlignCenter);
            homeLogoLabel->setStyleSheet("background: transparent;");
            homeLayout->addWidget(homeLogoLabel);
            homeLayout->addSpacing(12);
        }

        auto *welcomeLabel = new QLabel("欢迎使用 SeatShuffler 座位抽取系统");
        welcomeLabel->setAlignment(Qt::AlignCenter);
        welcomeLabel->setStyleSheet("font-size: 22px;"
                                    "font-weight: bold;"
                                    "color: #1f1f1f;"
                                    "background: transparent;");
        homeLayout->addWidget(welcomeLabel);

        auto *hintLabel = new QLabel("请使用上方导航栏选择功能模块");
        hintLabel->setAlignment(Qt::AlignCenter);
        hintLabel->setStyleSheet("font-size: 14px;"
                                 "color: #8c8c8c;"
                                 "background: transparent;"
                                 "margin-top: 8px;");
        homeLayout->addWidget(hintLabel);
        homeLayout->addStretch();

        // 创建各功能页面
        m_personPage = new PersonGroupListPage(this);
        m_classroomPage = new ClassroomListPage(this);
        m_rulePage = new RuleListPage(this);
        m_shufflePage = new ShufflePage(this);

        // 将常驻页面添加到 stack
        m_stack->addWidget(m_homePage); // index 0
        m_stack->addWidget(m_personPage); // index 1
        m_stack->addWidget(m_classroomPage); // index 2
        m_stack->addWidget(m_rulePage); // index 3
        m_stack->addWidget(m_shufflePage); // index 4

        m_currentBasePage = m_homePage;
        m_stack->setCurrentWidget(m_homePage);
    }

    void MainWindow::setupNavBar() {
        auto makeBtn = [this](const QString &text) -> QPushButton * {
            auto *btn = new QPushButton(text);
            btn->setCheckable(true);
            btn->setMinimumWidth(90);
            btn->setFixedHeight(34);
            btn->setCursor(Qt::PointingHandCursor);
            btn->setStyleSheet("QPushButton {"
                               "  background-color: transparent;"
                               "  border: none;"
                               "  border-radius: 6px;"
                               "  padding: 4px 16px;"
                               "  font-size: 13px;"
                               "  color: #595959;"
                               "}"
                               "QPushButton:hover {"
                               "  background-color: #f0f0f0;"
                               "  color: #1677ff;"
                               "}"
                               "QPushButton:checked {"
                               "  background-color: #e6f4ff;"
                               "  color: #1677ff;"
                               "  font-weight: bold;"
                               "}");
            m_navButtons.append(btn);
            return btn;
        };

        m_btnShuffle = makeBtn("编座位");
        m_btnPerson = makeBtn("人员管理");
        m_btnClassroom = makeBtn("教室管理");
        m_btnRule = makeBtn("规则管理");

        connect(m_btnShuffle, &QPushButton::clicked, this, &MainWindow::showShufflePage);
        connect(m_btnPerson, &QPushButton::clicked, this, &MainWindow::showPersonGroupPage);
        connect(m_btnClassroom, &QPushButton::clicked, this, &MainWindow::showClassroomPage);
        connect(m_btnRule, &QPushButton::clicked, this, &MainWindow::showRulePage);
    }

    void MainWindow::highlightNavButton(QPushButton *btn) {
        for (auto *b: m_navButtons) {
            b->setChecked(b == btn);
        }
    }

    // ============================================================
    // 页面切换
    // ============================================================

    void MainWindow::showHomePage() {
        for (auto *p: m_tempPages) {
            m_stack->removeWidget(p);
            p->deleteLater();
        }
        m_tempPages.clear();

        m_currentBasePage = m_homePage;
        m_stack->setCurrentWidget(m_homePage);
        highlightNavButton(nullptr);
    }

    void MainWindow::showPersonGroupPage() {
        for (auto *p: m_tempPages) {
            m_stack->removeWidget(p);
            p->deleteLater();
        }
        m_tempPages.clear();

        m_currentBasePage = m_personPage;
        m_personPage->refresh();
        m_stack->setCurrentWidget(m_personPage);
        highlightNavButton(m_btnPerson);
    }

    void MainWindow::showClassroomPage() {
        for (auto *p: m_tempPages) {
            m_stack->removeWidget(p);
            p->deleteLater();
        }
        m_tempPages.clear();

        m_currentBasePage = m_classroomPage;
        m_classroomPage->refresh();
        m_stack->setCurrentWidget(m_classroomPage);
        highlightNavButton(m_btnClassroom);
    }

    void MainWindow::showRulePage() {
        for (auto *p: m_tempPages) {
            m_stack->removeWidget(p);
            p->deleteLater();
        }
        m_tempPages.clear();

        m_currentBasePage = m_rulePage;
        m_rulePage->refresh();
        m_stack->setCurrentWidget(m_rulePage);
        highlightNavButton(m_btnRule);
    }

    void MainWindow::showShufflePage() {
        for (auto *p: m_tempPages) {
            m_stack->removeWidget(p);
            p->deleteLater();
        }
        m_tempPages.clear();

        m_currentBasePage = m_shufflePage;
        m_shufflePage->refresh();
        m_stack->setCurrentWidget(m_shufflePage);
        highlightNavButton(m_btnShuffle);
    }

    // ============================================================
    // 页面栈操作
    // ============================================================

    void MainWindow::pushPage(QWidget *page) {
        m_tempPages.append(page);
        m_stack->addWidget(page);
        m_stack->setCurrentWidget(page);
    }

    void MainWindow::popPage() {
        if (m_tempPages.isEmpty())
            return;

        auto *top = m_tempPages.takeLast();
        m_stack->removeWidget(top);
        top->deleteLater();

        if (!m_tempPages.isEmpty()) {
            m_stack->setCurrentWidget(m_tempPages.last());
        } else if (m_currentBasePage) {
            m_stack->setCurrentWidget(m_currentBasePage);

            if (m_currentBasePage == m_personPage)
                m_personPage->refresh();
            if (m_currentBasePage == m_classroomPage)
                m_classroomPage->refresh();
            if (m_currentBasePage == m_rulePage)
                m_rulePage->refresh();
            if (m_currentBasePage == m_shufflePage)
                m_shufflePage->refresh();
        }
    }

} // namespace seat
