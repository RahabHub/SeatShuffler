/**
 * @file ClassroomGridWidget.cpp
 * @brief 教室网格控件实现
 *
 * 在 QWidget 上绘制教室的二维网格，
 * 支持鼠标点击/拖拽来切换单元格类型。
 * 网格水平居中显示，顶部带"讲台方向"标签。
 */

#include "ClassroomGridWidget.h"
#include <QMouseEvent>
#include <QPainter>
#include <algorithm>

namespace seat {

    static constexpr int MIN_CELL_SIZE = 30;
    static constexpr int MAX_CELL_SIZE = 60;
    static constexpr int GRID_PADDING = 10;
    static constexpr int TOP_RESERVE = 24; ///< "讲台方向" 标签预留高度

    ClassroomGridWidget::ClassroomGridWidget(QWidget *parent) : QWidget(parent) {
        setMouseTracking(true);
        setMinimumSize(200, 200);
    }

    void ClassroomGridWidget::setClassroom(Classroom *classroom) {
        m_classroom = classroom;
        update();
    }

    int ClassroomGridWidget::cellSize() const {
        if (!m_classroom || m_classroom->rows == 0 || m_classroom->cols == 0) {
            return MIN_CELL_SIZE;
        }

        const int availW = width() - 2 * GRID_PADDING;
        const int availH = height() - 2 * GRID_PADDING - TOP_RESERVE;

        const int sizeByW = availW / m_classroom->cols;
        const int sizeByH = availH / m_classroom->rows;
        int s = std::min(sizeByW, sizeByH);

        return std::clamp(s, MIN_CELL_SIZE, MAX_CELL_SIZE);
    }

    /** 计算网格的水平居中偏移量 */
    int ClassroomGridWidget::gridOffsetX() const {
        if (!m_classroom)
            return GRID_PADDING;
        const int cs = cellSize();
        const int gridW = m_classroom->cols * cs;
        return std::max(GRID_PADDING, (width() - gridW) / 2);
    }

    /** 网格的垂直起始坐标 (为讲台标签预留空间) */
    int ClassroomGridWidget::gridOffsetY() const { return GRID_PADDING + TOP_RESERVE; }

    QPair<int, int> ClassroomGridWidget::cellAt(const QPoint &pos) const {
        if (!m_classroom)
            return {-1, -1};

        const int cs = cellSize();
        const int ox = gridOffsetX();
        const int oy = gridOffsetY();

        const int col = (pos.x() - ox) / cs;
        const int row = (pos.y() - oy) / cs;

        if (row >= 0 && row < m_classroom->rows && col >= 0 && col < m_classroom->cols)
            return {row, col};
        return {-1, -1};
    }

    void ClassroomGridWidget::applyBrush(int row, int col) {
        if (!m_classroom)
            return;
        if (row < 0 || row >= m_classroom->rows || col < 0 || col >= m_classroom->cols)
            return;

        CellType target = CellType::Empty;
        switch (m_brush) {
            case BrushType::Seat:
                target = CellType::Seat;
                break;
            case BrushType::Aisle:
                target = CellType::Aisle;
                break;
            case BrushType::Empty:
                target = CellType::Empty;
                break;
        }

        if (m_classroom->grid[row][col] != target) {
            m_classroom->grid[row][col] = target;
            emit gridModified();
            update();
        }
    }

    QSize ClassroomGridWidget::sizeHint() const {
        if (!m_classroom)
            return {400, 300};
        int cs = MIN_CELL_SIZE;
        return {m_classroom->cols * cs + 2 * GRID_PADDING,
                m_classroom->rows * cs + 2 * GRID_PADDING + TOP_RESERVE + 30};
    }

    // ============================================================
    // 绘制
    // ============================================================

    void ClassroomGridWidget::paintEvent(QPaintEvent *) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        if (!m_classroom) {
            painter.setPen(QColor("#8c8c8c"));
            painter.drawText(rect(), Qt::AlignCenter, "无教室数据");
            return;
        }

        const int cs = cellSize();
        const int ox = gridOffsetX();
        const int oy = gridOffsetY();

        // Ant Design 风格配色
        static const QColor colorSeat("#bae0ff");
        static const QColor colorAisle("#fafafa");
        static const QColor colorEmpty("#d9d9d9");
        static const QColor colorBorder("#e8e8e8");
        static const QColor colorSeatText("#0958d9");
        static const QColor colorLabelText("#8c8c8c");

        // ─── 绘制"讲台方向"标签 (在网格正上方) ───
        const int gridW = m_classroom->cols * cs;
        painter.setPen(colorLabelText);
        QFont labelFont = painter.font();
        labelFont.setPointSize(10);
        painter.setFont(labelFont);
        painter.drawText(QRect(ox, GRID_PADDING, gridW, TOP_RESERVE - 4), Qt::AlignHCenter | Qt::AlignBottom,
                         "← 讲台方向 →");

        // ─── 绘制网格 (居中) ──────────────────────
        for (int r = 0; r < m_classroom->rows; ++r) {
            for (int c = 0; c < m_classroom->cols; ++c) {
                QRect cellRect(ox + c * cs, oy + r * cs, cs, cs);

                QColor fillColor;
                switch (m_classroom->grid[r][c]) {
                    case CellType::Seat:
                        fillColor = colorSeat;
                        break;
                    case CellType::Aisle:
                        fillColor = colorAisle;
                        break;
                    case CellType::Empty:
                        fillColor = colorEmpty;
                        break;
                }

                // 圆角单元格
                painter.setPen(Qt::NoPen);
                painter.setBrush(fillColor);
                painter.drawRoundedRect(cellRect.adjusted(1, 1, -1, -1), 4, 4);

                painter.setPen(colorBorder);
                painter.setBrush(Qt::NoBrush);
                painter.drawRoundedRect(cellRect.adjusted(1, 1, -1, -1), 4, 4);

                // 在座位格子中标注行列号
                if (m_classroom->grid[r][c] == CellType::Seat) {
                    painter.setPen(colorSeatText);
                    QFont f = painter.font();
                    f.setPointSize(std::max(7, cs / 5));
                    painter.setFont(f);
                    painter.drawText(cellRect, Qt::AlignCenter, QString("%1,%2").arg(r + 1).arg(c + 1));
                }
            }
        }

        // ─── 底部图例 (居中) ──────────────────────
        const int legendY = oy + m_classroom->rows * cs + 14;
        QFont lf = painter.font();
        lf.setPointSize(10);
        painter.setFont(lf);

        const int legendTotalW = 240;
        const int legendStartX = std::max(GRID_PADDING, (width() - legendTotalW) / 2);

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

        drawLegend(legendStartX, colorSeat, "座位");
        drawLegend(legendStartX + 80, colorAisle, "过道");
        drawLegend(legendStartX + 160, colorEmpty, "空位");
    }

    // ============================================================
    // 鼠标交互
    // ============================================================

    void ClassroomGridWidget::mousePressEvent(QMouseEvent *event) {
        if (event->button() == Qt::LeftButton) {
            m_dragging = true;
            auto [row, col] = cellAt(event->pos());
            applyBrush(row, col);
        }
    }

    void ClassroomGridWidget::mouseMoveEvent(QMouseEvent *event) {
        if (m_dragging) {
            auto [row, col] = cellAt(event->pos());
            applyBrush(row, col);
        }
    }

    void ClassroomGridWidget::mouseReleaseEvent(QMouseEvent *event) {
        if (event->button() == Qt::LeftButton) {
            m_dragging = false;
        }
    }

} // namespace seat
