/**
 * @file RuleListPage.cpp
 * @brief 规则列表页面实现
 *
 * 以卡片形式展示所有规则，支持新建、删除、编辑。
 * 点击编辑进入 RuleEditorPage 编辑约束。
 */

#include "RuleListPage.h"
#include "MainWindow.h"
#include "RuleEditorPage.h"
#include "src/storage/DataManager.h"
#include "src/ui/common/AppStyle.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>

namespace seat {

    RuleListPage::RuleListPage(MainWindow *mainWindow, QWidget *parent) : QWidget(parent), m_mainWindow(mainWindow) {
        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(24, 20, 24, 20);
        layout->setSpacing(16);

        // ─── 标题栏 ────────────────────────────────
        auto *headerLayout = new QHBoxLayout();
        auto *titleLabel = new QLabel("规则管理");
        titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #1f1f1f; background: transparent;");
        headerLayout->addWidget(titleLabel);
        headerLayout->addStretch();

        auto *btnCreate = new QPushButton("+ 新建规则");
        AppStyle::applyPrimaryStyle(btnCreate);
        btnCreate->setCursor(Qt::PointingHandCursor);
        connect(btnCreate, &QPushButton::clicked, this, &RuleListPage::onCreateRule);
        headerLayout->addWidget(btnCreate);
        layout->addLayout(headerLayout);

        // ─── 可滚动卡片区域 ────────────────────────
        auto *scrollArea = new QScrollArea();
        scrollArea->setWidgetResizable(true);
        scrollArea->setFrameShape(QFrame::NoFrame);

        m_cardsContainer = new QWidget();
        m_cardsContainer->setStyleSheet("background: transparent;");
        m_cardsLayout = new QVBoxLayout(m_cardsContainer);
        m_cardsLayout->setAlignment(Qt::AlignTop);
        m_cardsLayout->setSpacing(10);
        scrollArea->setWidget(m_cardsContainer);
        layout->addWidget(scrollArea, 1);
    }

    void RuleListPage::refresh() { rebuildCards(); }

    void RuleListPage::rebuildCards() {
        QLayoutItem *child;
        while ((child = m_cardsLayout->takeAt(0)) != nullptr) {
            if (child->widget())
                child->widget()->deleteLater();
            delete child;
        }

        auto rules = DataManager::instance().loadAllRules();

        if (rules.isEmpty()) {
            auto *emptyLabel = new QLabel("暂无规则，点击右上角「新建规则」创建");
            emptyLabel->setAlignment(Qt::AlignCenter);
            emptyLabel->setStyleSheet("color: #8c8c8c; padding: 60px; font-size: 14px; background: transparent;");
            m_cardsLayout->addWidget(emptyLabel);
            return;
        }

        for (const auto &rule: rules) {
            auto *card = new QFrame();
            card->setFrameShape(QFrame::StyledPanel);

            auto *cardLayout = new QHBoxLayout(card);
            cardLayout->setContentsMargins(16, 12, 16, 12);

            auto *infoLayout = new QVBoxLayout();
            infoLayout->setSpacing(4);

            auto *nameLabel = new QLabel(rule.name);
            nameLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #1f1f1f; background: transparent;");
            infoLayout->addWidget(nameLabel);

            auto *metaLabel = new QLabel(
                    QString("创建时间: %1  |  约束数: %2").arg(rule.createdTime).arg(rule.constraints.size()));
            metaLabel->setStyleSheet("color: #8c8c8c; font-size: 12px; background: transparent;");
            infoLayout->addWidget(metaLabel);
            cardLayout->addLayout(infoLayout, 1);

            QString ruleId = rule.id;

            auto *btnDelete = new QPushButton("删除");
            btnDelete->setFixedWidth(64);
            btnDelete->setCursor(Qt::PointingHandCursor);
            AppStyle::applyDangerStyle(btnDelete);
            connect(btnDelete, &QPushButton::clicked, this, [this, ruleId]() { onDeleteRule(ruleId); });
            cardLayout->addWidget(btnDelete);

            auto *btnEdit = new QPushButton("编辑");
            btnEdit->setFixedWidth(64);
            btnEdit->setCursor(Qt::PointingHandCursor);
            connect(btnEdit, &QPushButton::clicked, this, [this, ruleId]() { onOpenRule(ruleId); });
            cardLayout->addWidget(btnEdit);

            m_cardsLayout->addWidget(card);
        }
    }

    void RuleListPage::onCreateRule() {
        bool ok = false;
        QString name = QInputDialog::getText(this, "新建规则", "请输入规则名称:", QLineEdit::Normal, "", &ok);
        if (!ok || name.trimmed().isEmpty())
            return;

        Rule rule;
        rule.id = generateId();
        rule.name = name.trimmed();
        rule.createdTime = currentTimestamp();

        DataManager::instance().saveRule(rule);
        rebuildCards();
    }

    void RuleListPage::onDeleteRule(const QString &id) {
        auto rule = DataManager::instance().loadRule(id);
        auto ret = QMessageBox::question(this, "确认删除", QString("确定要删除规则「%1」吗？").arg(rule.name));
        if (ret != QMessageBox::Yes)
            return;

        DataManager::instance().deleteRule(id);
        rebuildCards();
    }

    void RuleListPage::onOpenRule(const QString &id) const {
        auto *editorPage = new RuleEditorPage(id, m_mainWindow);
        m_mainWindow->pushPage(editorPage);
    }

} // namespace seat
