/**
 * @file ClassroomGridWidget.h
 * @brief 教室网格可视化控件
 *
 * 在一个 QWidget 上绘制教室的二维网格，
 * 支持鼠标点击/拖拽来切换单元格类型。
 * 网格水平居中显示，顶部带"讲台方向"标签。
 *
 * 颜色约定:
 *   - 座位 (Seat):  浅蓝色
 *   - 过道 (Aisle): 近白
 *   - 空位 (Empty): 灰色
 */

#ifndef SEATSHUFFLER_CLASSROOMGRIDWIDGET_H
#define SEATSHUFFLER_CLASSROOMGRIDWIDGET_H

#include <QWidget>
#include "src/models/Classroom.h"

namespace seat {

    class ClassroomGridWidget : public QWidget {
        Q_OBJECT

    public:
        explicit ClassroomGridWidget(QWidget *parent = nullptr);
        ~ClassroomGridWidget() override = default;

        /** 设置教室数据 (引用，外部拥有所有权) */
        void setClassroom(Classroom *classroom);

        /** 获取关联的教室指针 */
        Classroom *classroom() const { return m_classroom; }

        /** 绘制模式 */
        enum class BrushType { Seat, Aisle, Empty };
        void setBrush(BrushType brush) { m_brush = brush; }
        BrushType brush() const { return m_brush; }

    signals:
        void gridModified();

    protected:
        void paintEvent(QPaintEvent *event) override;
        void mousePressEvent(QMouseEvent *event) override;
        void mouseMoveEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;
        QSize sizeHint() const override;

    private:
        /** 将鼠标坐标转换为网格 (row, col)，无效时返回 (-1, -1) */
        QPair<int, int> cellAt(const QPoint &pos) const;

        /** 计算单元格的像素尺寸 */
        int cellSize() const;

        /** 计算网格水平居中偏移量 */
        int gridOffsetX() const;

        /** 计算网格垂直起始坐标 (为讲台标签预留空间) */
        int gridOffsetY() const;

        /** 应用当前画刷到指定单元格 */
        void applyBrush(int row, int col);

        Classroom *m_classroom = nullptr;
        BrushType m_brush = BrushType::Aisle;
        bool m_dragging = false;
    };

} // namespace seat

#endif // SEATSHUFFLER_CLASSROOMGRIDWIDGET_H
