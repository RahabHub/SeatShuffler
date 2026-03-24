/**
 * @file ClassroomListPage.cpp
 * @brief 教室列表页面实现
 *
 * 以卡片形式展示所有教室，支持新建、删除、编辑。
 * 点击编辑进入 ClassroomEditorPage 编辑教室布局。
 */

#include "ClassroomListPage.h"
#include "ClassroomEditorPage.h"
#include "MainWindow.h"
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

    ClassroomListPage::ClassroomListPage(MainWindow *mainWindow, QWidget *parent) :
        QWidget(parent), m_mainWindow(mainWindow) {
        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(24, 20, 24, 20);
        layout->setSpacing(16);

        // ─── 标题栏 ────────────────────────────────
        auto *headerLayout = new QHBoxLayout();
        auto *titleLabel = new QLabel("教室管理");
        titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #1f1f1f; background: transparent;");
        headerLayout->addWidget(titleLabel);
        headerLayout->addStretch();

        auto *btnCreate = new QPushButton("+ 新建教室");
        AppStyle::applyPrimaryStyle(btnCreate);
        btnCreate->setCursor(Qt::PointingHandCursor);
        connect(btnCreate, &QPushButton::clicked, this, &ClassroomListPage::onCreateClassroom);
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

    void ClassroomListPage::refresh() { rebuildCards(); }

    void ClassroomListPage::rebuildCards() {
        QLayoutItem *child;
        while ((child = m_cardsLayout->takeAt(0)) != nullptr) {
            if (child->widget())
                child->widget()->deleteLater();
            delete child;
        }

        auto classrooms = DataManager::instance().loadAllClassrooms();

        if (classrooms.isEmpty()) {
            auto *emptyLabel = new QLabel("暂无教室，点击右上角「新建教室」创建");
            emptyLabel->setAlignment(Qt::AlignCenter);
            emptyLabel->setStyleSheet("color: #8c8c8c; padding: 60px; font-size: 14px; background: transparent;");
            m_cardsLayout->addWidget(emptyLabel);
            return;
        }

        for (const auto &room: classrooms) {
            auto *card = new QFrame();
            card->setFrameShape(QFrame::StyledPanel);

            auto *cardLayout = new QHBoxLayout(card);
            cardLayout->setContentsMargins(16, 12, 16, 12);

            auto *infoLayout = new QVBoxLayout();
            infoLayout->setSpacing(4);

            auto *nameLabel = new QLabel(room.name);
            nameLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #1f1f1f; background: transparent;");
            infoLayout->addWidget(nameLabel);

            auto *metaLabel = new QLabel(QString("创建时间: %1  |  %2行 × %3列  |  座位数: %4")
                                                 .arg(room.createdTime)
                                                 .arg(room.rows)
                                                 .arg(room.cols)
                                                 .arg(room.seatCount()));
            metaLabel->setStyleSheet("color: #8c8c8c; font-size: 12px; background: transparent;");
            infoLayout->addWidget(metaLabel);
            cardLayout->addLayout(infoLayout, 1);

            QString roomId = room.id;

            auto *btnDelete = new QPushButton("删除");
            btnDelete->setFixedWidth(64);
            btnDelete->setCursor(Qt::PointingHandCursor);
            AppStyle::applyDangerStyle(btnDelete);
            connect(btnDelete, &QPushButton::clicked, this, [this, roomId]() { onDeleteClassroom(roomId); });
            cardLayout->addWidget(btnDelete);

            auto *btnEdit = new QPushButton("编辑");
            btnEdit->setFixedWidth(64);
            btnEdit->setCursor(Qt::PointingHandCursor);
            connect(btnEdit, &QPushButton::clicked, this, [this, roomId]() { onOpenClassroom(roomId); });
            cardLayout->addWidget(btnEdit);

            m_cardsLayout->addWidget(card);
        }
    }

    void ClassroomListPage::onCreateClassroom() {
        bool ok = false;
        const QString name = QInputDialog::getText(this, "新建教室", "请输入教室名称:", QLineEdit::Normal, "", &ok);
        if (!ok || name.trimmed().isEmpty())
            return;

        Classroom room;
        room.id = generateId();
        room.name = name.trimmed();
        room.createdTime = currentTimestamp();
        room.rows = 6;
        room.cols = 8;
        room.initGrid();

        DataManager::instance().saveClassroom(room);
        rebuildCards();
    }

    void ClassroomListPage::onDeleteClassroom(const QString &id) {
        auto room = DataManager::instance().loadClassroom(id);
        auto ret = QMessageBox::question(this, "确认删除", QString("确定要删除教室「%1」吗？").arg(room.name));
        if (ret != QMessageBox::Yes)
            return;

        DataManager::instance().deleteClassroom(id);
        rebuildCards();
    }

    void ClassroomListPage::onOpenClassroom(const QString &id) const {
        auto *editorPage = new ClassroomEditorPage(id, m_mainWindow);
        m_mainWindow->pushPage(editorPage);
    }

} // namespace seat
