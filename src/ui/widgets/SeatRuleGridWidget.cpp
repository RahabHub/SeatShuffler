/**
 * @file SeatRuleGridWidget.cpp
 * @brief 规则编辑专用的座位网格控件实现
 *
 * 支持鼠标拖拽多选:
 *   - 点击空白座位开始选中
 *   - 拖拽经过的座位自动纳入选中
 *   - 点击已选中座位取消选中
 */

#include "SeatRuleGridWidget.h"
#include <QMouseEvent>
#include <QPainter>
#include <algorithm>

namespace seat {

    static constexpr int CELL_SIZE_MIN = 42;
    static constexpr int CELL_SIZE_MAX = 64;
    static constexpr int PADDING = 10;
    static constexpr int TOP_RESERVE = 24;

    SeatRuleGridWidget::SeatRuleGridWidget(QWidget *parent) : QWidget(parent) {
        setMinimumSize(200, 200);
        setMouseTracking(true);
    }

    void SeatRuleGridWidget::setClassroom(const Classroom &room) {
        m_room = room;
        // 只有 id 非空且 grid 已初始化才视为有效教室
        m_hasRoom = !room.id.isEmpty() && !room.grid.isEmpty();
        m_selected.clear();
        m_seatLabels.clear();
        update();
        emit selectionChanged();
    }

    void SeatRuleGridWidget::updateConstraintOverlay(const QVector<Constraint> &constraints) {
        m_seatLabels.clear();
        for (const auto &c: constraints) {
            if (c.type == ConstraintType::SeatFieldMatch) {
                int r = c.params["row"].toInt();
                int col = c.params["col"].toInt();
                QString val = c.params["value"].toString();
                auto key = qMakePair(r, col);
                if (m_seatLabels.contains(key))
                    m_seatLabels[key] += "\n" + val;
                else
                    m_seatLabels[key] = val;
            }
        }
        update();
    }

    QVector<QPair<int, int>> SeatRuleGridWidget::selectedSeats() const {
        QVector<QPair<int, int>> result;
        result.reserve(m_selected.size());
        for (const auto &s: m_selected)
            result.append(s);
        std::sort(result.begin(), result.end());
        return result;
    }

    void SeatRuleGridWidget::clearSelection() {
        m_selected.clear();
        update();
        emit selectionChanged();
    }

    // ============================================================
    // 布局计算
    // ============================================================

    int SeatRuleGridWidget::cellSize() const {
        if (!m_hasRoom || m_room.rows == 0 || m_room.cols == 0)
            return CELL_SIZE_MIN;
        const int availW = width() - 2 * PADDING;
        const int availH = height() - 2 * PADDING - TOP_RESERVE - 30;
        const int byW = availW / m_room.cols;
        const int byH = availH / m_room.rows;
        return std::clamp(std::min(byW, byH), CELL_SIZE_MIN, CELL_SIZE_MAX);
    }

    int SeatRuleGridWidget::gridOffsetX() const {
        if (!m_hasRoom)
            return PADDING;
        return std::max(PADDING, (width() - m_room.cols * cellSize()) / 2);
    }

    int SeatRuleGridWidget::gridOffsetY() const { return PADDING + TOP_RESERVE; }

    QPair<int, int> SeatRuleGridWidget::cellAt(const QPoint &pos) const {
        if (!m_hasRoom)
            return {-1, -1};
        const int cs = cellSize();
        const int col = (pos.x() - gridOffsetX()) / cs;
        const int row = (pos.y() - gridOffsetY()) / cs;
        if (row >= 0 && row < m_room.rows && col >= 0 && col < m_room.cols)
            return {row, col};
        return {-1, -1};
    }

    QSize SeatRuleGridWidget::sizeHint() const {
        if (!m_hasRoom)
            return {400, 300};
        int cs = CELL_SIZE_MIN;
        return {m_room.cols * cs + 2 * PADDING, m_room.rows * cs + 2 * PADDING + TOP_RESERVE + 30};
    }

    // ============================================================
    // 绘制
    // ============================================================

    void SeatRuleGridWidget::paintEvent(QPaintEvent *) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::TextAntialiasing, true);

        if (!m_hasRoom) {
            painter.setPen(QColor("#8c8c8c"));
            painter.drawText(rect(), Qt::AlignCenter, "请先选择关联教室");
            return;
        }

        const int cs = cellSize();
        const int ox = gridOffsetX();
        const int oy = gridOffsetY();

        static const QColor colorSeat("#bae0ff");
        static const QColor colorAisle("#fafafa");
        static const QColor colorEmpty("#d9d9d9");
        static const QColor colorBorder("#e8e8e8");
        static const QColor colorConstrained("#efdbff");
        static const QColor colorSelected("#1677ff");
        static const QColor colorSeatText("#0958d9");
        static const QColor colorConstraintText("#531dab");

        // 讲台标签
        painter.setPen(QColor("#8c8c8c"));
        QFont labelFont = painter.font();
        labelFont.setPointSize(10);
        painter.setFont(labelFont);
        const int gridW = m_room.cols * cs;
        painter.drawText(QRect(ox, PADDING, gridW, TOP_RESERVE - 4), Qt::AlignHCenter | Qt::AlignBottom,
                         "← 讲台方向 →");

        // 绘制网格
        for (int r = 0; r < m_room.rows; ++r) {
            for (int c = 0; c < m_room.cols; ++c) {
                QRect cellRect(ox + c * cs, oy + r * cs, cs, cs);
                auto key = qMakePair(r, c);
                CellType type = m_room.grid[r][c];
                bool isSeat = (type == CellType::Seat);

                QColor fill;
                if (type == CellType::Aisle)
                    fill = colorAisle;
                else if (type == CellType::Empty)
                    fill = colorEmpty;
                else if (m_seatLabels.contains(key))
                    fill = colorConstrained;
                else
                    fill = colorSeat;

                painter.setPen(Qt::NoPen);
                painter.setBrush(fill);
                painter.drawRoundedRect(cellRect.adjusted(1, 1, -1, -1), 4, 4);

                painter.setPen(colorBorder);
                painter.setBrush(Qt::NoBrush);
                painter.drawRoundedRect(cellRect.adjusted(1, 1, -1, -1), 4, 4);

                if (isSeat) {
                    if (m_seatLabels.contains(key)) {
                        painter.setPen(colorConstraintText);
                        QFont f = painter.font();
                        f.setPointSize(std::max(8, cs / 5));
                        f.setBold(true);
                        painter.setFont(f);
                        QString label = m_seatLabels[key];
                        QFontMetrics fm(painter.font());
                        label = fm.elidedText(label, Qt::ElideRight, cs - 6);
                        painter.drawText(cellRect, Qt::AlignCenter, label);
                    } else {
                        painter.setPen(colorSeatText);
                        QFont f = painter.font();
                        f.setPointSize(std::max(7, cs / 6));
                        f.setBold(false);
                        painter.setFont(f);
                        painter.drawText(cellRect, Qt::AlignCenter, QString("%1,%2").arg(r + 1).arg(c + 1));
                    }
                }

                // 选中高亮
                if (isSeat && m_selected.contains(key)) {
                    QPen selPen(colorSelected, 2.5, Qt::DashLine);
                    painter.setPen(selPen);
                    painter.setBrush(Qt::NoBrush);
                    painter.drawRoundedRect(cellRect.adjusted(2, 2, -2, -2), 3, 3);
                }
            }
        }

        // 底部提示
        int legendY = oy + m_room.rows * cs + 10;
        painter.setPen(QColor("#8c8c8c"));
        QFont hf = painter.font();
        hf.setPointSize(10);
        hf.setBold(false);
        painter.setFont(hf);
        painter.drawText(QRect(ox, legendY, gridW, 20), Qt::AlignCenter,
                         QString("点击或拖拽选中座位  |  已选中 %1 个座位").arg(m_selected.size()));
    }

    // ============================================================
    // 鼠标交互: 点击 + 拖拽多选
    // ============================================================

    void SeatRuleGridWidget::mousePressEvent(QMouseEvent *event) {
        if (event->button() != Qt::LeftButton || !m_hasRoom)
            return;

        auto [row, col] = cellAt(event->pos());
        if (row < 0 || col < 0)
            return;
        if (m_room.grid[row][col] != CellType::Seat)
            return;

        auto key = qMakePair(row, col);
        m_dragging = true;

        // 首次点击决定本次拖拽是"选中"还是"取消"
        if (m_selected.contains(key)) {
            m_dragMode = false; // 取消模式
            m_selected.remove(key);
        } else {
            m_dragMode = true; // 选中模式
            m_selected.insert(key);
        }

        update();
        emit selectionChanged();
    }

    void SeatRuleGridWidget::mouseMoveEvent(QMouseEvent *event) {
        if (!m_dragging || !m_hasRoom)
            return;

        auto [row, col] = cellAt(event->pos());
        if (row < 0 || col < 0)
            return;
        if (m_room.grid[row][col] != CellType::Seat)
            return;

        auto key = qMakePair(row, col);
        bool changed = false;

        if (m_dragMode) {
            // 选中模式: 拖过的座位全部选中
            if (!m_selected.contains(key)) {
                m_selected.insert(key);
                changed = true;
            }
        } else {
            // 取消模式: 拖过的座位全部取消
            if (m_selected.contains(key)) {
                m_selected.remove(key);
                changed = true;
            }
        }

        if (changed) {
            update();
            emit selectionChanged();
        }
    }

    void SeatRuleGridWidget::mouseReleaseEvent(QMouseEvent *event) {
        if (event->button() == Qt::LeftButton)
            m_dragging = false;
    }

} // namespace seat
