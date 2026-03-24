/**
 * @file DataManager.cpp
 * @brief DataManager 实现
 */

#include "DataManager.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QStandardPaths>

namespace seat {

    // 子目录名称常量
    static constexpr auto DIR_GROUPS = "person_groups";
    static constexpr auto DIR_CLASSROOMS = "classrooms";
    static constexpr auto DIR_RULES = "rules";
    static constexpr auto DIR_RECORDS = "records";

    // ============================================================
    // 单例
    // ============================================================

    DataManager &DataManager::instance() {
        static DataManager inst;
        return inst;
    }

    DataManager::DataManager(QObject *parent) : QObject(parent) {

        // 数据存储在应用程序可写目录下的 SeatShuffler 子目录
        m_dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        if (m_dataPath.isEmpty()) {
            m_dataPath = QDir::currentPath() + "/data";
        }

        // 确保所有子目录存在
        ensureDirectory(DIR_GROUPS);
        ensureDirectory(DIR_CLASSROOMS);
        ensureDirectory(DIR_RULES);
        ensureDirectory(DIR_RECORDS);

        qDebug() << "[DataManager] Data Path:" << m_dataPath;
    }

    // ============================================================
    // PersonGroup
    // ============================================================

    QVector<PersonGroup> DataManager::loadAllGroups() const {
        QVector<PersonGroup> result;
        const QDir dir(m_dataPath + "/" + DIR_GROUPS);
        for (const auto files = dir.entryList({"*.json"}, QDir::Files); const auto &f: files) {
            if (auto obj = readJsonFile(dir.filePath(f)); !obj.isEmpty()) {
                result.append(PersonGroup::fromJson(obj));
            }
        }
        return result;
    }

    PersonGroup DataManager::loadGroup(const QString &id) const {
        const auto obj = readJsonFile(filePath(DIR_GROUPS, id));
        return obj.isEmpty() ? PersonGroup{} : PersonGroup::fromJson(obj);
    }

    bool DataManager::saveGroup(const PersonGroup &group) {
        const bool ok = writeJsonFile(filePath(DIR_GROUPS, group.id), group.toJson());
        if (ok)
            emit groupsChanged();
        return ok;
    }

    bool DataManager::deleteGroup(const QString &id) {
        const bool ok = removeFile(filePath(DIR_GROUPS, id));
        if (ok)
            emit groupsChanged();
        return ok;
    }

    // ============================================================
    // Classroom
    // ============================================================

    QVector<Classroom> DataManager::loadAllClassrooms() const {
        QVector<Classroom> result;
        const QDir dir(m_dataPath + "/" + DIR_CLASSROOMS);
        for (const auto files = dir.entryList({"*.json"}, QDir::Files); const auto &f: files) {
            if (auto obj = readJsonFile(dir.filePath(f)); !obj.isEmpty()) {
                result.append(Classroom::fromJson(obj));
            }
        }
        return result;
    }

    Classroom DataManager::loadClassroom(const QString &id) const {
        const auto obj = readJsonFile(filePath(DIR_CLASSROOMS, id));
        return obj.isEmpty() ? Classroom{} : Classroom::fromJson(obj);
    }

    bool DataManager::saveClassroom(const Classroom &classroom) {
        const bool ok = writeJsonFile(filePath(DIR_CLASSROOMS, classroom.id), classroom.toJson());
        if (ok)
            emit classroomsChanged();
        return ok;
    }

    bool DataManager::deleteClassroom(const QString &id) {
        const bool ok = removeFile(filePath(DIR_CLASSROOMS, id));
        if (ok)
            emit classroomsChanged();
        return ok;
    }

    // ============================================================
    // Rule
    // ============================================================

    QVector<Rule> DataManager::loadAllRules() const {
        QVector<Rule> result;
        const QDir dir(m_dataPath + "/" + DIR_RULES);
        for (const auto files = dir.entryList({"*.json"}, QDir::Files); const auto &f: files) {
            if (auto obj = readJsonFile(dir.filePath(f)); !obj.isEmpty()) {
                result.append(Rule::fromJson(obj));
            }
        }
        return result;
    }

    Rule DataManager::loadRule(const QString &id) const {
        const auto obj = readJsonFile(filePath(DIR_RULES, id));
        return obj.isEmpty() ? Rule{} : Rule::fromJson(obj);
    }

    bool DataManager::saveRule(const Rule &rule) {
        const bool ok = writeJsonFile(filePath(DIR_RULES, rule.id), rule.toJson());
        if (ok)
            emit rulesChanged();
        return ok;
    }

    bool DataManager::deleteRule(const QString &id) {
        const bool ok = removeFile(filePath(DIR_RULES, id));
        if (ok)
            emit rulesChanged();
        return ok;
    }

    // ============================================================
    // ShuffleRecord
    // ============================================================

    QVector<ShuffleRecord> DataManager::loadAllRecords() const {
        QVector<ShuffleRecord> result;
        const QDir dir(m_dataPath + "/" + DIR_RECORDS);
        for (const auto files = dir.entryList({"*.json"}, QDir::Files); const auto &f: files) {
            if (auto obj = readJsonFile(dir.filePath(f)); !obj.isEmpty()) {
                result.append(ShuffleRecord::fromJson(obj));
            }
        }
        return result;
    }

    ShuffleRecord DataManager::loadRecord(const QString &id) const {
        const auto obj = readJsonFile(filePath(DIR_RECORDS, id));
        return obj.isEmpty() ? ShuffleRecord{} : ShuffleRecord::fromJson(obj);
    }

    bool DataManager::saveRecord(const ShuffleRecord &record) {
        const bool ok = writeJsonFile(filePath(DIR_RECORDS, record.id), record.toJson());
        if (ok) emit recordsChanged();
        return ok;
    }

    bool DataManager::deleteRecord(const QString &id) {
        const bool ok = removeFile(filePath(DIR_RECORDS, id));
        if (ok) emit recordsChanged();
        return ok;
    }

    // ============================================================
    // 内部工具函数
    // ============================================================

    void DataManager::ensureDirectory(const QString &subdir) const {
        if (const QDir dir(m_dataPath + "/" + subdir); !dir.exists()) {
            const bool success = dir.mkpath(".");
            Q_UNUSED(success);
        }
    }

    QString DataManager::filePath(const QString &subdir, const QString &id) const {
        return m_dataPath + "/" + subdir + "/" + id + ".json";
    }

    QJsonObject DataManager::readJsonFile(const QString &filePath) {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "[DataManager] Cannot open file for reading:" << filePath;
            return {};
        }
        QJsonParseError err;
        auto doc = QJsonDocument::fromJson(file.readAll(), &err);
        if (err.error != QJsonParseError::NoError) {
            qWarning() << "[DataManager] JSON parse error:" << err.errorString() << "in" << filePath;
            return {};
        }
        return doc.object();
    }

    bool DataManager::writeJsonFile(const QString &filePath, const QJsonObject &obj) {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            qWarning() << "[DataManager] Cannot open file for writing:" << filePath;
            return false;
        }
        file.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
        return true;
    }

    bool DataManager::removeFile(const QString &filePath) {
        if (QFile::exists(filePath)) {
            return QFile::remove(filePath);
        }
        return true; // 文件不存在也视为删除成功
    }

} // namespace seat
