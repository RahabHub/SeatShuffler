/**
 * @file ShuffleEngine.h
 * @brief 座位抽取引擎
 *
 * 核心优化:
 *   - 预分配: 唯一可确定人员的约束先行处理，大幅减少搜索空间
 *   - 回溯法处理剩余人员分配
 */

#ifndef SEATSHUFFLER_SHUFFLEENGINE_H
#define SEATSHUFFLER_SHUFFLEENGINE_H

#include <QString>
#include "src/models/Classroom.h"
#include "src/models/PersonGroup.h"
#include "src/models/Rule.h"
#include "src/models/ShuffleRecord.h"

namespace seat {

    class ShuffleEngine {
    public:
        enum class ResultStatus { Success, TooManyPersons, NoSolution, InvalidInput };

        struct Result {
            ResultStatus status;
            QString errorMessage;
            ShuffleRecord record;
        };

        struct SeatPos {
            int row;
            int col;
        };

        static Result execute(const PersonGroup &group, const Classroom &room, const Rule &rule,
                              const QString &nameField = "");

    private:
        static bool checkConstraints(const QVector<int> &seatAssignment, const QVector<Person> &persons,
                                     const QVector<SeatPos> &seats, const Classroom &room,
                                     const QVector<Constraint> &constraints, bool partial);

        /**
         * @brief 回溯搜索 (使用间接索引列表)
         * @param step           当前处理到 pendingPersons 中的第几个
         * @param pendingPersons 待分配人员在 persons 中的索引列表
         */
        static bool backtrack(int step, const QVector<int> &pendingPersons, QVector<int> &seatAssignment,
                              QVector<bool> &seatUsed, const QVector<Person> &persons, const QVector<SeatPos> &seats,
                              const Classroom &room, const QVector<Constraint> &constraints, int &attempts,
                              int maxAttempts);
    };

} // namespace seat

#endif // SEATSHUFFLER_SHUFFLEENGINE_H
