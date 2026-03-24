/**
 * @file HistoryDetailPage.h
 * @brief 历史记录详情页面
 *
 * 展示单条抽取记录的详细信息和座位网格，
 * 支持导出为图片 (PNG) 和表格 (CSV)。
 */

#ifndef SEATSHUFFLER_HISTORYDETAILPAGE_H
#define SEATSHUFFLER_HISTORYDETAILPAGE_H

#include <QWidget>
#include "src/models/ShuffleRecord.h"

namespace seat {

    class MainWindow;
    class ResultGridWidget;

    class HistoryDetailPage final : public QWidget {
        Q_OBJECT

    public:
        explicit HistoryDetailPage(const QString &recordId, MainWindow *mainWindow, QWidget *parent = nullptr);
        ~HistoryDetailPage() override = default;

    private slots:
        void onBack() const;
        void onExportImage();
        void onExportCsv();

    private:
        MainWindow *m_mainWindow;
        ResultGridWidget *m_resultGrid = nullptr; ///< 网格控件指针 (用于图片导出)
        ShuffleRecord m_record; ///< 当前记录 (用于 CSV 导出)
        bool m_hasRoom = false; ///< 是否有对应教室数据
    };

} // namespace seat

#endif // SEATSHUFFLER_HISTORYDETAILPAGE_H
