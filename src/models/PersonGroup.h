/**
 * @file PersonGroup.h
 * @brief 人员组数据模型
 *
 * PersonGroup 表示一个人员组，包含:
 *   - 元信息 (id, name, createdTime)
 *   - 自定义列定义 (ColumnDef)
 *   - 人员数据列表 (Person)
 *
 * 每个 Person 拥有一个自动生成的 _id 字段，以及与列定义对应的键值对。
 * 支持与 QJsonObject 之间的互相转换，用于本地持久化。
 */

#ifndef SEATSHUFFLER_PERSONGROUP_H
#define SEATSHUFFLER_PERSONGROUP_H

#include <QJsonArray>
#include <QJsonObject>
#include <QMap>
#include <QString>
#include <QVariant>
#include <QVector>
#include "../ui/common/Common.h"

namespace seat {
    // ============================================================
    // ColumnDef — 自定义列定义
    // ============================================================

    /**
     * @brief 用户自定义的列定义
     *
     * 每一列有一个名称和类型。目前支持的类型为 "string"。
     * 后续可扩展为 "int", "enum" 等。
     */
    struct ColumnDef {
        QString name; // 列名（如“姓名”，“性别”）
        QString type = "string"; // 列类型，默认为字符串

        QJsonObject toJson() const { return {{"name", name}, {"type", type}}; }

        static ColumnDef fromJson(const QJsonObject &obj) {
            return {obj["name"].toString(), obj["type"].toString("string")};
        }
    };

    // ============================================================
    // Person — 单个人员
    // ============================================================

    /**
     * @brief 人员数据，键值对形式存储
     *
     * _id 为系统自动生成的唯一标识，不可修改
     * fields 存储与 ColumnDef 对应的数据
     */
    struct Person {
        QString id;
        QMap<QString, QString> fields;

        QJsonObject toJson() const {
            QJsonObject obj;
            obj["_id"] = id;
            for (auto it = fields.begin(); it != fields.end(); ++it) {
                obj[it.key()] = it.value();
            }
            return obj;
        }

        static Person fromJson(const QJsonObject &obj) {
            Person p;
            p.id = obj["_id"].toString();
            if (p.id.isEmpty()) {
                p.id = generateId();
            }
            for (auto it = obj.begin(); it != obj.end(); ++it) {
                if (it.key() != "_id") {
                    p.fields[it.key()] = it.value().toString();
                }
            }
            return p;
        }
    };

    // ============================================================
    // PersonGroup — 人员组
    // ============================================================

    struct PersonGroup {
        QString id; // 组唯一标识
        QString name; // 组名称
        QString createdTime; // 创建时间
        QString note; // 备注
        QVector<ColumnDef> columns; // 列定义
        QVector<Person> persons; // 人员定义
        int personCount = 0; // 人员数量（冗余字段）

        QJsonObject toJson() const {
            QJsonArray colArr, perArr;
            for (const auto &c: columns)
                colArr.append(c.toJson());
            for (const auto &p: persons)
                perArr.append(p.toJson());

            return {
                {"id", id},
                {"name", name},
                {"createdTime", createdTime},
                {"note", note},
                {"columns", colArr},
                {"persons", perArr}
            };
        }

        static PersonGroup fromJson(const QJsonObject &obj) {
            PersonGroup g;
            g.id = obj["id"].toString();
            g.name = obj["name"].toString();
            g.createdTime = obj["createdTime"].toString();
            g.note = obj["note"].toString();

            for (const auto &v:obj["columns"].toArray())
                g.columns.append(ColumnDef::fromJson(v.toObject()));
            for (const auto &v:obj["persons"].toArray())
                g.persons.append(Person::fromJson(v.toObject()));

            g.personCount = g.persons.size();
            return g;
        }
    };
} // namespace seat

#endif // SEATSHUFFLER_PERSONGROUP_H
