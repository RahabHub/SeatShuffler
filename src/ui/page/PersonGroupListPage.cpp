/**
 * @file PersonGroupListPage.cpp
 * @brief 人员组列表页面实现
 *
 * 以卡片形式展示所有人员组，支持新建、删除、编辑。
 * 点击编辑可跳转到 PersonGroupDetailPage 查看/编辑详情。
 */

#include "PersonGroupListPage.h"
#include "MainWindow.h"
#include "PersonGroupDetailPage.h"
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

    PersonGroupListPage::PersonGroupListPage(MainWindow *mainWindow, QWidget *parent) :
        QWidget(parent), m_mainWindow(mainWindow) {
        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(24, 20, 24, 20);
        layout->setSpacing(16);

        // ─── 标题栏 ────────────────────────────────
        auto *headerLayout = new QHBoxLayout();
        auto *titleLabel = new QLabel("人员组管理");
        titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #1f1f1f; background: transparent;");
        headerLayout->addWidget(titleLabel);
        headerLayout->addStretch();

        auto *btnCreate = new QPushButton("+ 新建人员组");
        AppStyle::applyPrimaryStyle(btnCreate);
        btnCreate->setCursor(Qt::PointingHandCursor);
        connect(btnCreate, &QPushButton::clicked, this, &PersonGroupListPage::onCreateGroup);
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

    void PersonGroupListPage::refresh() { rebuildCards(); }

    void PersonGroupListPage::rebuildCards() {
        // 清空旧卡片
        QLayoutItem *child;
        while ((child = m_cardsLayout->takeAt(0)) != nullptr) {
            if (child->widget())
                child->widget()->deleteLater();
            delete child;
        }

        // 加载数据并创建卡片
        auto groups = DataManager::instance().loadAllGroups();

        if (groups.isEmpty()) {
            auto *emptyLabel = new QLabel("暂无人员组，点击右上角「新建人员组」创建");
            emptyLabel->setAlignment(Qt::AlignCenter);
            emptyLabel->setStyleSheet("color: #8c8c8c; padding: 60px; font-size: 14px; background: transparent;");
            m_cardsLayout->addWidget(emptyLabel);
            return;
        }

        for (const auto &group: groups) {
            auto *card = new QFrame();
            card->setFrameShape(QFrame::StyledPanel);
            card->setCursor(Qt::PointingHandCursor);

            auto *cardLayout = new QHBoxLayout(card);
            cardLayout->setContentsMargins(16, 12, 16, 12);

            // 组信息
            auto *infoLayout = new QVBoxLayout();
            infoLayout->setSpacing(4);

            auto *nameLabel = new QLabel(group.name);
            nameLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #1f1f1f; background: transparent;");
            infoLayout->addWidget(nameLabel);

            auto *metaLabel = new QLabel(QString("创建时间: %1  |  人数: %2  |  列数: %3")
                                                 .arg(group.createdTime, QString::number(group.persons.size()),
                                                      QString::number(group.columns.size())));
            metaLabel->setStyleSheet("color: #8c8c8c; font-size: 12px; background: transparent;");
            infoLayout->addWidget(metaLabel);

            if (!group.note.isEmpty()) {
                auto *noteLabel = new QLabel("备注: " + group.note);
                noteLabel->setStyleSheet("color: #a0a0a0; font-size: 11px; background: transparent;");
                infoLayout->addWidget(noteLabel);
            }

            cardLayout->addLayout(infoLayout, 1);

            // 操作按钮
            QString groupId = group.id;

            auto *btnDelete = new QPushButton("删除");
            btnDelete->setFixedWidth(64);
            btnDelete->setCursor(Qt::PointingHandCursor);
            AppStyle::applyDangerStyle(btnDelete);
            connect(btnDelete, &QPushButton::clicked, this, [this, groupId]() { onDeleteGroup(groupId); });
            cardLayout->addWidget(btnDelete);

            auto *btnEdit = new QPushButton("编辑");
            btnEdit->setFixedWidth(64);
            btnEdit->setCursor(Qt::PointingHandCursor);
            connect(btnEdit, &QPushButton::clicked, this, [this, groupId]() { onOpenGroup(groupId); });
            cardLayout->addWidget(btnEdit);

            m_cardsLayout->addWidget(card);
        }
    }

    // ============================================================
    // 槽函数
    // ============================================================

    void PersonGroupListPage::onCreateGroup() {
        bool ok = false;
        QString name = QInputDialog::getText(this, "新建人员组", "请输入人员组名称:", QLineEdit::Normal, "", &ok);
        if (!ok || name.trimmed().isEmpty())
            return;

        PersonGroup group;
        group.id = generateId();
        group.name = name.trimmed();
        group.createdTime = currentTimestamp();

        DataManager::instance().saveGroup(group);
        rebuildCards();
    }

    void PersonGroupListPage::onDeleteGroup(const QString &id) {
        auto group = DataManager::instance().loadGroup(id);
        auto ret = QMessageBox::question(this, "确认删除",
                                         QString("确定要删除人员组「%1」吗？此操作不可恢复。").arg(group.name));
        if (ret != QMessageBox::Yes)
            return;

        DataManager::instance().deleteGroup(id);
        rebuildCards();
    }

    void PersonGroupListPage::onOpenGroup(const QString &id) const {
        auto *detailPage = new PersonGroupDetailPage(id, m_mainWindow);
        m_mainWindow->pushPage(detailPage);
    }

} // namespace seat
