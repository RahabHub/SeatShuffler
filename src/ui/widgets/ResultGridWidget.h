/**
 * @file ResultGridWidget.h
 * @brief 抽取结果网格显示控件
 *
 * 根据教室布局和分配结果，在网格中显示每个座位上的人员姓名。
 * 支持将结果渲染为 QPixmap 用于图片导出。
 */
#ifndef SEATSHUFFLER_RESULTGRIDWIDGET_H
#define SEATSHUFFLER_RESULTGRIDWIDGET_H

#include <QPixmap>
#include <QWidget>
#include "src/models/Classroom.h"
#include "src/models/ShuffleRecord.h"

namespace seat {

    class ResultGridWidget : public QWidget {
        Q_OBJECT

    public:
        explicit ResultGridWidget(QWidget *parent = nullptr);
        ~ResultGridWidget() override = default;

        /** 设置要显示的数据 */
        void setData(const Classroom &room, const ShuffleRecord &record);

        /** 清除显示 */
        void clear();

        /**
         * @brief 将当前网格渲染为 QPixmap (用于图片导出)
         * @return 包含完整网格绘制内容的图片，如无数据则返回空 QPixmap
         */
        QPixmap renderToPixmap() const;

    protected:
        void paintEvent(QPaintEvent *event) override;
        QSize sizeHint() const override;

    private:
        static int cellWidth();
        static int cellHeight();

        Classroom m_room;
        ShuffleRecord m_record;
        bool m_hasData = false;

        /** (row,col) -> personName 映射，用于快速查找 */
        QMap<QPair<int, int>, QString> m_nameMap;
    };
} // namespace seat

#endif // SEATSHUFFLER_RESULTGRIDWIDGET_H
