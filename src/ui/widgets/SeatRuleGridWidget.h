/**
 * @file SeatRuleGridWidget.h
 * @brief 规则编辑专用的座位网格控件
 *
 * 支持:
 *   - 点击切换座位选中/取消
 *   - 拖拽批量选中/取消
 *   - 已设约束的座位以颜色和文字标注
 */

#ifndef SEATSHUFFLER_SEATRULEGRIDWIDGET_H
#define SEATSHUFFLER_SEATRULEGRIDWIDGET_H

#include <QMap>
#include <QPair>
#include <QSet>
#include <QWidget>
#include "src/models/Classroom.h"
#include "src/models/Rule.h"

namespace seat {

    class SeatRuleGridWidget : public QWidget {
        Q_OBJECT

    public:
        explicit SeatRuleGridWidget(QWidget *parent = nullptr);
        ~SeatRuleGridWidget() override = default;

        void setClassroom(const Classroom &room);
        void updateConstraintOverlay(const QVector<Constraint> &constraints);
        QVector<QPair<int, int>> selectedSeats() const;
        int selectedCount() const { return m_selected.size(); }
        void clearSelection();

    signals:
        void selectionChanged();

    protected:
        void paintEvent(QPaintEvent *event) override;
        void mousePressEvent(QMouseEvent *event) override;
        void mouseMoveEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;
        QSize sizeHint() const override;

    private:
        QPair<int, int> cellAt(const QPoint &pos) const;
        int cellSize() const;
        int gridOffsetX() const;
        int gridOffsetY() const;

        Classroom m_room;
        bool m_hasRoom = false;

        QSet<QPair<int, int>> m_selected;
        QMap<QPair<int, int>, QString> m_seatLabels;

        // ─── 拖拽选择状态 ─────────────────────────
        bool m_dragging = false; ///< 是否正在拖拽
        bool m_dragMode = true; ///< true=选中模式, false=取消模式
    };

} // namespace seat

#endif // SEATSHUFFLER_SEATRULEGRIDWIDGET_H
