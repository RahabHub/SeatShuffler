/**
 * @file ResultGridWidget.cpp
 * @brief 结果网格控件实现
 *
 * 根据教室布局和分配结果，在网格中显示每个座位上的人员姓名。
 * 网格水平居中，顶部显示讲台方向标签。
 */

#include "ResultGridWidget.h"
#include <QPainter>
#include <algorithm>
#include <qevent.h>

namespace seat {

    static constexpr int CELL_W = 72;
    static constexpr int CELL_H = 36;
    static constexpr int PADDING = 12;
    static constexpr int TOP_RESERVE = 28;

    ResultGridWidget::ResultGridWidget(QWidget *parent) : QWidget(parent) { setMinimumSize(200, 150); }

    void ResultGridWidget::setData(const Classroom &room, const ShuffleRecord &record) {
        m_room = room;
        m_record = record;
        m_hasData = true;

        m_nameMap.clear();
        for (const auto &a: record.assignments) {
            m_nameMap[{a.row, a.col}] = a.personName;
        }
        update();
    }

    void ResultGridWidget::clear() {
        m_hasData = false;
        m_nameMap.clear();
        update();
    }

    int ResultGridWidget::cellWidth() { return CELL_W; }
    int ResultGridWidget::cellHeight() { return CELL_H; }

    QSize ResultGridWidget::sizeHint() const {
        if (!m_hasData)
            return {400, 300};

        const int gridW = m_room.cols * CELL_W;
        const int gridH = m_room.rows * CELL_H;

        return {gridW + 2 * PADDING, TOP_RESERVE + PADDING + gridH + 40};
    }

    void ResultGridWidget::paintEvent(QPaintEvent *) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::TextAntialiasing, true);

        if (!m_hasData) {
            painter.setPen(QColor("#8c8c8c"));
            QFont f = painter.font();
            f.setPointSize(13);
            painter.setFont(f);
            painter.drawText(rect(), Qt::AlignCenter, "尚未执行抽取");
            return;
        }

        static const QColor colorSeat("#bae0ff");
        static const QColor colorOccupied("#b7eb8f");
        static const QColor colorAisle("#fafafa");
        static const QColor colorEmpty("#f0f0f0");
        static const QColor colorBorder("#e8e8e8");
        static const QColor colorNameText("#1f1f1f");
        static const QColor colorLabelText("#8c8c8c");

        const int cw = cellWidth();
        const int ch = cellHeight();

        // ─── 水平居中偏移 ─────────────────────────
        const int gridTotalW = m_room.cols * cw;
        const int offsetX = std::max(PADDING, (width() - gridTotalW) / 2);
        const int offsetY = TOP_RESERVE;

        // ─── 讲台方向标签 ─────────────────────────
        painter.setPen(colorLabelText);
        QFont labelFont = painter.font();
        labelFont.setPointSize(11);
        painter.setFont(labelFont);
        painter.drawText(QRect(offsetX, 4, gridTotalW, TOP_RESERVE - 6), Qt::AlignHCenter | Qt::AlignBottom,
                         "← 讲台方向 →");

        // ─── 绘制网格 ─────────────────────────────
        for (int r = 0; r < m_room.rows; ++r) {
            for (int c = 0; c < m_room.cols; ++c) {
                QRect cellRect(offsetX + c * cw, offsetY + r * ch, cw, ch);

                CellType type = m_room.grid[r][c];
                QColor fill;

                if (type == CellType::Aisle)
                    fill = colorAisle;
                else if (type == CellType::Empty)
                    fill = colorEmpty;
                else {
                    auto key = qMakePair(r, c);
                    fill = m_nameMap.contains(key) ? colorOccupied : colorSeat;
                }

                painter.setPen(Qt::NoPen);
                painter.setBrush(fill);
                painter.drawRoundedRect(cellRect.adjusted(1, 1, -1, -1), 4, 4);

                painter.setPen(colorBorder);
                painter.setBrush(Qt::NoBrush);
                painter.drawRoundedRect(cellRect.adjusted(1, 1, -1, -1), 4, 4);

                if (type == CellType::Seat) {
                    auto key = qMakePair(r, c);
                    if (m_nameMap.contains(key)) {
                        painter.setPen(colorNameText);
                        QFont f = painter.font();
                        f.setPointSize(std::max(8, ch / 4));
                        f.setBold(true);
                        painter.setFont(f);

                        QString name = m_nameMap[key];
                        QFontMetrics fm(painter.font());
                        QString elided = fm.elidedText(name, Qt::ElideRight, cw - 6);
                        painter.drawText(cellRect, Qt::AlignCenter, elided);
                    }
                }
            }
        }

        // ─── 图例 (居中) ──────────────────────────
        const int legendY = offsetY + m_room.rows * ch + 14;
        QFont lf = painter.font();
        lf.setPointSize(10);
        lf.setBold(false);
        painter.setFont(lf);

        const int legendTotalW = 270;
        const int legendStartX = std::max(PADDING, (width() - legendTotalW) / 2);

        auto drawLegend = [&](int x, const QColor &color, const QString &text) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(color);
            painter.drawRoundedRect(x, legendY, 16, 16, 3, 3);
            painter.setPen(colorBorder);
            painter.setBrush(Qt::NoBrush);
            painter.drawRoundedRect(x, legendY, 16, 16, 3, 3);
            painter.setPen(QColor("#595959"));
            painter.drawText(x + 22, legendY + 13, text);
        };

        drawLegend(legendStartX, colorOccupied, "已分配");
        drawLegend(legendStartX + 90, colorSeat, "空座位");
        drawLegend(legendStartX + 180, colorAisle, "过道");
    }

    // ============================================================
    // 导出为图片
    // ============================================================

    QPixmap ResultGridWidget::renderToPixmap() const {
        if (!m_hasData)
            return {};

        // 计算理想画布尺寸
        const int cw = cellWidth();
        const int ch = cellHeight();
        const int gridW = m_room.cols * cw;
        const int gridH = m_room.rows * ch;
        const int margin = 20;
        const int topReserve = 32;
        const int bottomReserve = 40;

        const int imgW = gridW + 2 * margin;
        const int imgH = topReserve + gridH + bottomReserve + 2 * margin;

        QPixmap pixmap(imgW, imgH);
        pixmap.fill(Qt::white);

        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::TextAntialiasing, true);

        static const QColor colorSeat("#bae0ff");
        static const QColor colorOccupied("#b7eb8f");
        static const QColor colorAisle("#fafafa");
        static const QColor colorEmpty("#f0f0f0");
        static const QColor colorBorder("#e8e8e8");
        static const QColor colorNameText("#1f1f1f");
        static const QColor colorLabelText("#8c8c8c");

        const int offsetX = margin;
        const int offsetY = margin + topReserve;

        // 讲台标签
        painter.setPen(colorLabelText);
        QFont labelFont("Microsoft YaHei UI", 11);
        painter.setFont(labelFont);
        painter.drawText(QRect(offsetX, margin, gridW, topReserve - 4), Qt::AlignHCenter | Qt::AlignBottom,
                         "← 讲台方向 →");

        // 网格
        for (int r = 0; r < m_room.rows; ++r) {
            for (int c = 0; c < m_room.cols; ++c) {
                QRect cellRect(offsetX + c * cw, offsetY + r * ch, cw, ch);

                CellType type = m_room.grid[r][c];
                QColor fill;

                if (type == CellType::Aisle)
                    fill = colorAisle;
                else if (type == CellType::Empty)
                    fill = colorEmpty;
                else {
                    auto key = qMakePair(r, c);
                    fill = m_nameMap.contains(key) ? colorOccupied : colorSeat;
                }

                painter.setPen(Qt::NoPen);
                painter.setBrush(fill);
                painter.drawRoundedRect(cellRect.adjusted(1, 1, -1, -1), 4, 4);

                painter.setPen(colorBorder);
                painter.setBrush(Qt::NoBrush);
                painter.drawRoundedRect(cellRect.adjusted(1, 1, -1, -1), 4, 4);

                if (type == CellType::Seat) {
                    auto key = qMakePair(r, c);
                    if (m_nameMap.contains(key)) {
                        painter.setPen(colorNameText);
                        QFont f("Microsoft YaHei UI", std::max(8, ch / 4));
                        f.setBold(true);
                        painter.setFont(f);

                        QString name = m_nameMap[key];
                        QFontMetrics fm(painter.font());
                        QString elided = fm.elidedText(name, Qt::ElideRight, cw - 6);
                        painter.drawText(cellRect, Qt::AlignCenter, elided);
                    }
                }
            }
        }

        // 图例
        const int legendY = offsetY + gridH + 14;
        QFont lf("Microsoft YaHei UI", 10);
        painter.setFont(lf);

        const int legendStartX = (imgW - 270) / 2;

        auto drawLegend = [&](int x, const QColor &color, const QString &text) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(color);
            painter.drawRoundedRect(x, legendY, 16, 16, 3, 3);
            painter.setPen(colorBorder);
            painter.setBrush(Qt::NoBrush);
            painter.drawRoundedRect(x, legendY, 16, 16, 3, 3);
            painter.setPen(QColor("#595959"));
            painter.drawText(x + 22, legendY + 13, text);
        };

        drawLegend(legendStartX, colorOccupied, "已分配");
        drawLegend(legendStartX + 90, colorSeat, "空座位");
        drawLegend(legendStartX + 180, colorAisle, "过道");

        painter.end();
        return pixmap;
    }

} // namespace seat
