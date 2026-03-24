/**
 * @file HistoryDetailPage.cpp
 * @brief 历史记录详情页实现
 *
 * 展示单条抽取记录的详细信息，
 * 若教室仍存在则以网格形式展示，否则以列表展示。
 * 支持导出为 PNG 图片和 CSV 表格。
 *
 * 修复:
 *   - 返回按钮首次渲染时的焦点框线 (通过 setFocusPolicy 清除)
 *   - 新增导出图片和导出 CSV 功能
 */

#include "HistoryDetailPage.h"
#include "MainWindow.h"
#include "src/storage/DataManager.h"
#include "src/ui/common/AppStyle.h"
#include "src/ui/widgets/ResultGridWidget.h"

#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QTextStream>
#include <QVBoxLayout>

namespace seat {

    HistoryDetailPage::HistoryDetailPage(const QString &recordId, MainWindow *mainWindow, QWidget *parent) :
        QWidget(parent), m_mainWindow(mainWindow) {
        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(24, 20, 24, 20);
        layout->setSpacing(14);

        // ─── 顶部: 返回 + 导出按钮 ───────────────
        auto *topBar = new QHBoxLayout();

        auto *btnBack = new QPushButton("← 返回");
        btnBack->setCursor(Qt::PointingHandCursor);
        btnBack->setFocusPolicy(Qt::NoFocus); // 消除首次渲染时的焦点框线
        connect(btnBack, &QPushButton::clicked, this, &HistoryDetailPage::onBack);
        topBar->addWidget(btnBack);

        topBar->addStretch();

        auto *btnExportImg = new QPushButton("导出图片");
        btnExportImg->setCursor(Qt::PointingHandCursor);
        connect(btnExportImg, &QPushButton::clicked, this, &HistoryDetailPage::onExportImage);
        topBar->addWidget(btnExportImg);

        auto *btnExportCsv = new QPushButton("导出表格");
        btnExportCsv->setCursor(Qt::PointingHandCursor);
        connect(btnExportCsv, &QPushButton::clicked, this, &HistoryDetailPage::onExportCsv);
        topBar->addWidget(btnExportCsv);

        layout->addLayout(topBar);

        // ─── 加载记录 ─────────────────────────────
        m_record = DataManager::instance().loadRecord(recordId);

        if (m_record.id.isEmpty()) {
            auto *notFoundLabel = new QLabel("记录未找到");
            notFoundLabel->setStyleSheet("color: #ff4d4f; font-size: 14px; background: transparent;");
            layout->addWidget(notFoundLabel);

            // 禁用导出按钮
            btnExportImg->setEnabled(false);
            btnExportCsv->setEnabled(false);
            return;
        }

        // ─── 元信息卡片 ───────────────────────────
        auto *infoLabel = new QLabel(
                QString("时间: %1\n人员组: %2\n教室: %3\n规则: %4\n分配人数: %5")
                        .arg(m_record.timestamp, m_record.groupName, m_record.classroomName, m_record.ruleName)
                        .arg(m_record.assignments.size()));
        infoLabel->setStyleSheet("background-color: #ffffff;"
                                 "border: 1px solid #e8e8e8;"
                                 "border-radius: 8px;"
                                 "padding: 14px 18px;"
                                 "color: #1f1f1f;"
                                 "font-size: 13px;");
        layout->addWidget(infoLabel);

        // ─── 尝试加载对应教室来显示网格 ────────────
        Classroom room;
        if (!m_record.classroomId.isEmpty()) {
            room = DataManager::instance().loadClassroom(m_record.classroomId);
            if (!room.id.isEmpty()) {
                m_hasRoom = true;
            }
        }

        if (m_hasRoom) {
            auto *scrollArea = new QScrollArea();
            scrollArea->setWidgetResizable(true);

            m_resultGrid = new ResultGridWidget();
            m_resultGrid->setData(room, m_record);
            scrollArea->setWidget(m_resultGrid);
            layout->addWidget(scrollArea, 1);
        } else {
            // 教室已被删除，以列表形式展示分配
            auto *label = new QLabel("原教室已被删除，以列表形式展示:");
            label->setStyleSheet("color: #faad14; font-size: 13px; background: transparent;");
            layout->addWidget(label);

            // 网格导出不可用
            btnExportImg->setEnabled(false);

            QString text;
            for (const auto &a: m_record.assignments) {
                text += QString("座位(%1, %2): %3\n").arg(a.row + 1).arg(a.col + 1).arg(a.personName);
            }
            auto *textLabel = new QLabel(text);
            textLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
            textLabel->setStyleSheet("background-color: #ffffff;"
                                     "border: 1px solid #e8e8e8;"
                                     "border-radius: 8px;"
                                     "padding: 14px 18px;"
                                     "color: #1f1f1f;"
                                     "font-size: 13px;");
            auto *scroll = new QScrollArea();
            scroll->setWidgetResizable(true);
            scroll->setWidget(textLabel);
            layout->addWidget(scroll, 1);
        }
    }

    void HistoryDetailPage::onBack() const { m_mainWindow->popPage(); }

    // ============================================================
    // 导出为图片 (PNG)
    // ============================================================

    void HistoryDetailPage::onExportImage() {
        if (!m_resultGrid) {
            QMessageBox::information(this, "提示", "没有可导出的网格图");
            return;
        }

        // 使用 renderToPixmap 获得理想尺寸的渲染结果
        QPixmap pixmap = m_resultGrid->renderToPixmap();
        if (pixmap.isNull()) {
            QMessageBox::warning(this, "错误", "渲染图片失败");
            return;
        }

        // 生成默认文件名
        QString defaultName = QString("座位分配_%1.png").arg(m_record.timestamp.replace(':', '-'));

        QString filePath =
                QFileDialog::getSaveFileName(this, "导出图片", defaultName, "PNG 图片 (*.png);;所有文件 (*)");

        if (filePath.isEmpty())
            return;

        if (!filePath.endsWith(".png", Qt::CaseInsensitive)) {
            filePath += ".png";
        }

        if (pixmap.save(filePath, "PNG")) {
            QMessageBox::information(this, "导出成功", QString("图片已保存到:\n%1").arg(filePath));
        } else {
            QMessageBox::critical(this, "导出失败", "图片保存失败，请检查路径是否可写。");
        }
    }

    // ============================================================
    // 导出为表格 (CSV)
    // ============================================================

    void HistoryDetailPage::onExportCsv() {
        if (m_record.assignments.isEmpty()) {
            QMessageBox::information(this, "提示", "没有分配数据可导出");
            return;
        }

        QString defaultName = QString("座位分配_%1.csv").arg(m_record.timestamp.replace(':', '-'));

        QString filePath =
                QFileDialog::getSaveFileName(this, "导出表格", defaultName, "CSV 文件 (*.csv);;所有文件 (*)");

        if (filePath.isEmpty())
            return;

        if (!filePath.endsWith(".csv", Qt::CaseInsensitive)) {
            filePath += ".csv";
        }

        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this, "导出失败", "无法写入文件:\n" + filePath);
            return;
        }

        QTextStream stream(&file);

        // 写入 UTF-8 BOM 以便 Excel 正确识别中文
        stream << QChar(0xFEFF);

        // 写入元信息作为注释行
        stream << "# 时间," << m_record.timestamp << "\n";
        stream << "# 人员组," << m_record.groupName << "\n";
        stream << "# 教室," << m_record.classroomName << "\n";
        stream << "# 规则," << m_record.ruleName << "\n";

        // 表头
        stream << "行号,列号,人员ID,姓名\n";

        // 数据行
        for (const auto &a: m_record.assignments) {
            // CSV 转义: 如果字段包含逗号或引号，用双引号包裹
            auto csvEscape = [](const QString &s) -> QString {
                if (s.contains(',') || s.contains('"') || s.contains('\n')) {
                    QString escaped = s;
                    escaped.replace('"', "\"\"");
                    return '"' + escaped + '"';
                }
                return s;
            };

            stream << (a.row + 1) << "," << (a.col + 1) << "," << csvEscape(a.personId) << ","
                   << csvEscape(a.personName) << "\n";
        }

        file.close();

        QMessageBox::information(
                this, "导出成功",
                QString("CSV 文件已保存到:\n%1\n共 %2 条记录").arg(filePath).arg(m_record.assignments.size()));
    }

} // namespace seat
