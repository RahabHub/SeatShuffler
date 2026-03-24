/**
 * @file Common.h
 * @brief 公共类型定义与工具函数
 *
 * 包含整个项目通用的类型别名、枚举、常量等。
 */

#ifndef SEATSHUFFLER_COMMON_H
#define SEATSHUFFLER_COMMON_H

#include <QString>
#include <QUuid>
#include <QDateTime>

namespace seat {

    /** 生成一个不带花括号的 UUID 字符串，用作数据实体的唯一标识 */
    inline QString generateId() {
        return QUuid::createUuid().toString(QUuid::WithoutBraces);
    }

    /** 获取当前时间的 ISO 格式字符串 */
    inline QString currentTimestamp() {
        return QDateTime::currentDateTime().toString(Qt::ISODate);
    }
}

#endif //SEATSHUFFLER_COMMON_H