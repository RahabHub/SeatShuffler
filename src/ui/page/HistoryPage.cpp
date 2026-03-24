/**
 * @file HistoryPage.cpp
 * @brief 历史记录页面实现
 *
 * 展示所有抽取历史记录，按时间倒序排列，
 * 支持查看详情和删除。
 */

#include "HistoryPage.h"
#include "HistoryDetailPage.h"
#include "MainWindow.h"
#include "src/storage/DataManager.h"
#include "src/ui/common/AppStyle.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <algorithm>

namespace seat {

    HistoryPage::HistoryPage(MainWindow *mainWindow, QWidget *parent) : QWidget(parent), m_mainWindow(mainWindow) {
        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(24, 20, 24, 20);
        layout->setSpacing(16);

        // ─── 顶部 ─────────────────────────────────
        auto *topBar = new QHBoxLayout();
        auto *btnBack = new QPushButton("← 返回");
        btnBack->setCursor(Qt::PointingHandCursor);
        connect(btnBack, &QPushButton::clicked, this, &HistoryPage::onBack);
        topBar->addWidget(btnBack);

        auto *titleLabel = new QLabel("历史记录");
        titleLabel->setStyleSheet(
                "font-size: 18px; font-weight: bold; color: #1f1f1f; background: transparent; margin-left: 12px;");
        topBar->addWidget(titleLabel);
        topBar->addStretch();
        layout->addLayout(topBar);

        // ─── 列表 ─────────────────────────────────
        auto *scrollArea = new QScrollArea();
        scrollArea->setWidgetResizable(true);
        scrollArea->setFrameShape(QFrame::NoFrame);

        auto *container = new QWidget();
        container->setStyleSheet("background: transparent;");
        m_listLayout = new QVBoxLayout(container);
        m_listLayout->setAlignment(Qt::AlignTop);
        m_listLayout->setSpacing(10);
        scrollArea->setWidget(container);
        layout->addWidget(scrollArea, 1);

        rebuildList();
    }

    void HistoryPage::rebuildList() {
        QLayoutItem *child;
        while ((child = m_listLayout->takeAt(0)) != nullptr) {
            if (child->widget())
                child->widget()->deleteLater();
            delete child;
        }

        auto records = DataManager::instance().loadAllRecords();

        if (records.isEmpty()) {
            auto *emptyLabel = new QLabel("暂无历史记录");
            emptyLabel->setAlignment(Qt::AlignCenter);
            emptyLabel->setStyleSheet("color: #8c8c8c; padding: 60px; font-size: 14px; background: transparent;");
            m_listLayout->addWidget(emptyLabel);
            return;
        }

        // 按时间倒序排列
        std::sort(records.begin(), records.end(),
                  [](const ShuffleRecord &a, const ShuffleRecord &b) { return a.timestamp > b.timestamp; });

        for (const auto &rec: records) {
            auto *card = new QFrame();
            card->setFrameShape(QFrame::StyledPanel);

            auto *cardLayout = new QHBoxLayout(card);
            cardLayout->setContentsMargins(16, 12, 16, 12);

            auto *infoLayout = new QVBoxLayout();
            infoLayout->setSpacing(4);

            auto *timeLabel = new QLabel(rec.timestamp);
            timeLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #1f1f1f; background: transparent;");
            infoLayout->addWidget(timeLabel);

            auto *metaLabel = new QLabel(QString("人员组: %1  |  教室: %2  |  规则: %3  |  分配: %4 人")
                                                 .arg(rec.groupName, rec.classroomName, rec.ruleName)
                                                 .arg(rec.assignments.size()));
            metaLabel->setStyleSheet("color: #8c8c8c; font-size: 12px; background: transparent;");
            infoLayout->addWidget(metaLabel);
            cardLayout->addLayout(infoLayout, 1);

            QString recId = rec.id;

            auto *btnView = new QPushButton("查看");
            btnView->setFixedWidth(64);
            btnView->setCursor(Qt::PointingHandCursor);
            connect(btnView, &QPushButton::clicked, this, [this, recId]() { onViewRecord(recId); });
            cardLayout->addWidget(btnView);

            auto *btnDel = new QPushButton("删除");
            btnDel->setFixedWidth(64);
            btnDel->setCursor(Qt::PointingHandCursor);
            AppStyle::applyDangerStyle(btnDel);
            connect(btnDel, &QPushButton::clicked, this, [this, recId]() { onDeleteRecord(recId); });
            cardLayout->addWidget(btnDel);

            m_listLayout->addWidget(card);
        }
    }

    void HistoryPage::onBack() { m_mainWindow->popPage(); }

    void HistoryPage::onDeleteRecord(const QString &id) {
        auto ret = QMessageBox::question(this, "确认删除", "确定要删除这条历史记录吗？");
        if (ret != QMessageBox::Yes)
            return;

        DataManager::instance().deleteRecord(id);
        rebuildList();
    }

    void HistoryPage::onViewRecord(const QString &id) const {
        auto *detailPage = new HistoryDetailPage(id, m_mainWindow);
        m_mainWindow->pushPage(detailPage);
    }

} // namespace seat
