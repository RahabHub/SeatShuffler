/**
 * @file ShuffleRecord.h
 * @brief 抽取记录数据模型
 *
 * 保存每次抽取的结果，包含:
 *   - 使用的人员组、教室、规则的 id 与名称 (快照)
 *   - 座位分配结果 (row, col) -> personId
 */

#ifndef SEATSHUFFLER_SHUFFLERECORD_H
#define SEATSHUFFLER_SHUFFLERECORD_H

#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QVector>
#include "src/ui/common/Common.h"

namespace seat {

    /** 单个座位分配 */
    struct SeatAssignment {
        int row;
        int col;
        QString personId;
        QString personName;

        QJsonObject toJson() const {
            return {
                {"row", row}, {"col", col},
                {"personId", personId}, {"personName", personName}
            };
        }

        static SeatAssignment fromJson(const QJsonObject &obj) {
            return {
                obj["row"].toInt(), obj["col"].toInt(),
                obj["personId"].toString(), obj["personName"].toString()
            };
        }
    };

    /** 完整的抽取记录 */
    struct ShuffleRecord {
        QString id;
        QString timestamp;
        QString groupId;
        QString groupName;
        QString classroomId;
        QString classroomName;
        QString ruleId;
        QString ruleName;
        QVector<SeatAssignment> assignments;

        QJsonObject toJson() const {
            // 将所有座位分配序列化到 JSON 数组
            QJsonArray arr;
            for (const auto &a : assignments) {
                arr.append(a.toJson());
            }

            return {
                {"id", id}, {"timestamp", timestamp},
                {"groupId", groupId}, {"groupName", groupName},
                {"classroomId", classroomId}, {"classroomName", classroomName},
                {"ruleId", ruleId}, {"ruleName", ruleName},
                {"assignments", arr}
            };
        }

        static ShuffleRecord fromJson(const QJsonObject &obj) {
            ShuffleRecord r;
            r.id            = obj["id"].toString();
            r.timestamp     = obj["timestamp"].toString();
            r.groupId       = obj["groupId"].toString();
            r.groupName     = obj["groupName"].toString();
            r.classroomId   = obj["classroomId"].toString();
            r.classroomName = obj["classroomName"].toString();
            r.ruleId        = obj["ruleId"].toString();
            r.ruleName      = obj["ruleName"].toString();
            for (const auto &v : obj["assignments"].toArray())
                r.assignments.append(SeatAssignment::fromJson(v.toObject()));
            return r;
        }
    };
}

#endif // SEATSHUFFLER_SHUFFLERECORD_H