/**
 * @file Rule.h
 * @brief 规则数据模型
 *
 * 规则 (Rule) 由多个约束 (Constraint) 组成，可关联一个教室布局。
 *
 * ┌───────────────────────────────────────────────────────────┐
 * │ 约束类型           │ 说明                                 │
 * ├────────────────────┼──────────────────────────────────────┤
 * │ SeatFieldMatch     │ 指定座位的某字段必须为某值            │
 * │ RowPattern         │ 某一行按照模式串分配                  │
 * │ NoAdjacentSame     │ 同行相邻座位的某字段不能相同          │
 * │                    │ (空值默认跳过，不参与比较)            │
 * │ ForceAdjacentSame  │ 同行相邻座位的某字段必须相同          │
 * │                    │ (空值默认跳过，不参与比较)            │
 * │ ColumnFieldMatch   │ 指定列的所有座位的某字段必须为某值    │
 * │ ZoneFieldMatch     │ 矩形区域内座位的某字段必须为某值      │
 * │ ForbidPair         │ 两个特定人员不能坐相邻座位 (禁止同桌) │
 * │ ForcePair          │ 两个特定人员必须坐相邻座位 (强制同桌) │
 * └───────────────────────────────────────────────────────────┘
 */

#ifndef SEATSHUFFLER_RULE_H
#define SEATSHUFFLER_RULE_H

#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QVector>
#include "../ui/common/Common.h"

namespace seat {

    // ============================================================
    // ConstraintType — 约束类型枚举
    // ============================================================

    enum class ConstraintType : int {
        SeatFieldMatch = 0, // 指定座位字段匹配
        RowPattern = 1, // 行模式匹配
        NoAdjacentSame = 2, // 相邻座位字段不同 (空值跳过)
        ForceAdjacentSame = 3, // 相邻座位字段相同 (空值跳过)
        ColumnFieldMatch = 4, // 指定列字段匹配
        ZoneFieldMatch = 5, // 区域字段匹配
        ForbidPair = 6, // 禁止同桌 (两人不能相邻)
        ForcePair = 7 // 强制同桌 (两人必须相邻)
    };

    /** 约束类型 -> 中文可读名称 */
    inline QString constraintTypeName(ConstraintType t) {
        switch (t) {
            case ConstraintType::SeatFieldMatch:
                return "指定座位约束";
            case ConstraintType::RowPattern:
                return "行模式约束";
            case ConstraintType::NoAdjacentSame:
                return "相邻不同约束";
            case ConstraintType::ForceAdjacentSame:
                return "相邻相同约束";
            case ConstraintType::ColumnFieldMatch:
                return "指定列约束";
            case ConstraintType::ZoneFieldMatch:
                return "区域约束";
            case ConstraintType::ForbidPair:
                return "禁止同桌";
            case ConstraintType::ForcePair:
                return "强制同桌";
        }
        return "未知约束";
    }

    // ============================================================
    // Constraint — 单个约束
    // ============================================================

    /**
     * @brief 单个约束
     *
     * params 的结构取决于 type:
     *
     * SeatFieldMatch:
     *   { "row": int, "col": int, "field": "性别", "value": "男" }
     *
     * RowPattern:
     *   { "row": int, "field": "性别", "pattern": ["男","男","_","女","女"] }
     *
     * NoAdjacentSame / ForceAdjacentSame:
     *   { "field": "性别" }
     *   空字段值默认跳过比较
     *
     * ColumnFieldMatch:
     *   { "col": int, "field": "性别", "value": "男" }
     *
     * ZoneFieldMatch:
     *   { "rowStart": int, "rowEnd": int, "colStart": int, "colEnd": int,
     *     "field": "性别", "value": "男" }
     *
     * ForbidPair:
     *   { "personId1": "uuid1", "personName1": "张三",
     *     "personId2": "uuid2", "personName2": "李四" }
     *
     * ForcePair:
     *   同 ForbidPair 格式
     */
    struct Constraint {
        ConstraintType type;
        QJsonObject params;

        [[nodiscard]] QJsonObject toJson() const { return {{"type", static_cast<int>(type)}, {"params", params}}; }

        static Constraint fromJson(const QJsonObject &obj) {
            Constraint c;
            c.type = static_cast<ConstraintType>(obj["type"].toInt(0));
            c.params = obj["params"].toObject();
            return c;
        }
    };

    // ============================================================
    // Rule — 规则
    // ============================================================

    struct Rule {
        QString id;
        QString name;
        QString createdTime;
        QString note;
        QString classroomId; ///< 关联的教室 ID (可选，用于可视化编辑)
        QString referenceGroupId; ///< 参考人员组 ID (可选，用于人员对约束)
        QVector<Constraint> constraints;

        [[nodiscard]] QJsonObject toJson() const {
            QJsonArray arr;
            for (const auto &c: constraints)
                arr.append(c.toJson());
            return {{"id", id},
                    {"name", name},
                    {"createdTime", createdTime},
                    {"note", note},
                    {"classroomId", classroomId},
                    {"referenceGroupId", referenceGroupId},
                    {"constraints", arr}};
        }

        static Rule fromJson(const QJsonObject &obj) {
            Rule r;
            r.id = obj["id"].toString();
            r.name = obj["name"].toString();
            r.createdTime = obj["createdTime"].toString();
            r.note = obj["note"].toString();
            r.classroomId = obj["classroomId"].toString();
            r.referenceGroupId = obj["referenceGroupId"].toString();
            for (const auto &v: obj["constraints"].toArray())
                r.constraints.append(Constraint::fromJson(v.toObject()));
            return r;
        }
    };

} // namespace seat

#endif // SEATSHUFFLER_RULE_H
