/**
 * @file DataManager.h
 * @brief 数据持久化管理器
 *
 * 负责将所有数据实体 (PersonGroup, Classroom, Rule, ShuffleRecord)
 * 以 JSON 文件的形式存储到本地 data/ 目录。
 *
 * 目录结构:
 *   <appDataPath>/
 *   ├── person_groups/    每个组一个 JSON 文件
 *   ├── classrooms/       每个教室一个 JSON 文件
 *   ├── rules/            每个规则一个 JSON 文件
 *   └── records/          每条历史记录一个 JSON 文件
 *
 * DataManager 为单例模式，全局唯一实例。
 */

#ifndef SEATSHUFFLER_DATAMANAGER_H
#define SEATSHUFFLER_DATAMANAGER_H

#include <QObject>
#include <QString>
#include <QVector>
#include "src/models/Classroom.h"
#include "src/models/PersonGroup.h"
#include "src/models/Rule.h"
#include "src/models/ShuffleRecord.h"

namespace seat {
    class DataManager : public QObject {
        Q_OBJECT
    public:
        static DataManager &instance();

        /** 获取数据根目录路径 */
        [[nodiscard]] QString dataPath() const { return m_dataPath; }

        // ─── PersonGroup CRUD ───────────────────────────
        [[nodiscard]] QVector<PersonGroup> loadAllGroups() const;
        [[nodiscard]] PersonGroup loadGroup(const QString &id) const;
        bool saveGroup(const PersonGroup &group);
        bool deleteGroup(const QString &id);

        // ─── Classroom CRUD ─────────────────────────────
        [[nodiscard]] QVector<Classroom> loadAllClassrooms() const;
        [[nodiscard]] Classroom loadClassroom(const QString &id) const;
        bool saveClassroom(const Classroom &classroom);
        bool deleteClassroom(const QString &id);

        // ─── Rule CRUD ──────────────────────────────────
        [[nodiscard]] QVector<Rule> loadAllRules() const;
        [[nodiscard]] Rule loadRule(const QString &id) const;
        bool saveRule(const Rule &rule);
        bool deleteRule(const QString &id);

        // ─── ShuffleRecord CRUD ─────────────────────────
        [[nodiscard]] QVector<ShuffleRecord> loadAllRecords() const;
        [[nodiscard]] ShuffleRecord loadRecord(const QString &id) const;
        bool saveRecord(const ShuffleRecord &record);
        bool deleteRecord(const QString &id);

    signals:
        /** 数据变更通知，供 UI 刷新 */
        void groupsChanged();
        void classroomsChanged();
        void rulesChanged();
        void recordsChanged();

    private:
        explicit DataManager(QObject *parent = nullptr);
        ~DataManager() override = default;

        // 禁用拷贝
        DataManager(const DataManager &) = delete;
        DataManager &operator=(const DataManager &) = delete;

        /** 确保子目录存在 */
        void ensureDirectory(const QString &subdir) const;

        /** 通用的 JSON 读写 */
        static QJsonObject readJsonFile(const QString &filePath);
        static bool writeJsonFile(const QString &filePath, const QJsonObject &obj);
        static bool removeFile(const QString &filePath);

        /** 拼接子目录下的文件路径 */
        [[nodiscard]] QString filePath(const QString &subdir, const QString &id) const;

        QString m_dataPath; // 数据根目录
    };
} // namespace seat


#endif // SEATSHUFFLER_DATAMANAGER_H
