/**
 * @file Classroom.h
 * @brief 教室数据模型
 *
 * 教室用一个二维网格表示，每个格子可以是:
 *   - SEAT  : 座位
 *   - AISLE : 过道
 *   - EMPTY : 空位 (不放座位也不是过道)
 *
 * 用户可以自由调整网格尺寸，并通过拖拽来设置过道。
 */

#ifndef SEATSHUFFLER_CLASSROOM_H
#define SEATSHUFFLER_CLASSROOM_H

#include <QJsonArray>
#include <QJsonObject>
#include <QVector>
#include "../ui/common/Common.h"

namespace seat {
    // ============================================================
    // CellType —— 网格单元格类型
    // ============================================================
    enum class CellType : int { Seat = 0, Aisle = 1, Empty = 2 };

    // ============================================================
    // Classroom
    // ============================================================

    struct Classroom {
        QString id;
        QString name;
        QString createdTime;
        QString note;
        int rows = 6; // 网格行数（默认 6）
        int cols = 8; // 网格列数（默认 8）
        QVector<QVector<CellType>> grid; // 二维网格

        /** 初始化网格为全座位 */
        void initGrid() {
            grid.resize(rows);
            for (int r = 0; r < rows; ++r) {
                grid[r].fill(CellType::Seat, cols);
            }
        }

        /** 确保网格尺寸与 rows/cols 一致，扩展部分填充 Seat */
        void resizeGrid(const int newRows, const int newCols) {
            QVector<QVector<CellType>> newGrid(newRows);
            for (int r = 0; r < newRows; ++r) {
                newGrid[r].resize(newCols, CellType::Seat);
                if (r < rows) {
                    for (int c = 0; c < qMin(cols, newCols); ++c) {
                        newGrid[r][c] = grid[r][c];
                    }
                }
            }
            rows = newRows;
            cols = newCols;
            grid = std::move(newGrid);
        }

        /** 统计座位数量 */
        [[nodiscard]] int seatCount() const {
            int count = 0;
            for (const auto &row: grid)
                for (auto cell: row)
                        if (cell == CellType::Seat) ++count;
            return count;
        }

        [[nodiscard]] QJsonObject toJson() const {
            QJsonArray gridArr;
            for (const auto &row: grid) {
                QJsonArray rowArr;
                for (auto cell:row) rowArr.append(static_cast<int>(cell));
                gridArr.append(rowArr);
            }

            return {
            {"id", id}, {"name", name},
               {"createdTime", createdTime}, {"note", note},
                {"rows", rows}, {"cols", cols},
                {"grid", gridArr}
            };
        }

        static Classroom fromJson(const QJsonObject &obj) {
            Classroom c;
            c.id            = obj["id"].toString();
            c.name          = obj["name"].toString();
            c.createdTime   = obj["createdTime"].toString();
            c.note          = obj["note"].toString();
            c.rows          = obj["rows"].toInt(6);
            c.cols          = obj["cols"].toInt(8);

            if (const auto gridArr = obj["grid"].toArray(); gridArr.isEmpty()) {
                c.initGrid();
            } else {
                c.grid.resize(c.rows);
                for (int r = 0;r < c.rows && r < gridArr.size();++r) {
                    const auto rowArr = gridArr[r].toArray();
                    c.grid[r].resize(c.cols, CellType::Seat);
                    for (int col = 0;col < c.cols && col < rowArr.size(); ++col) {
                        c.grid[r][col] = static_cast<CellType>(rowArr[col].toInt(0));
                    }
                }
            }
            return c;
        }
    };

} // namespace seat

#endif // SEATSHUFFLER_CLASSROOM_H
