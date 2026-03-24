/**
 * @file ShuffleEngine.cpp
 * @brief 座位抽取引擎实现
 *
 * 核心优化:
 *   - 预分配: SeatFieldMatch 中能唯一确定人员的约束先行分配到指定座位
 *     (如 姓名=张强 在整个人员组中唯一 → 直接分配，不走回溯)
 *   - 回溯法处理剩余人员
 *   - 空字段跳过: NoAdjacentSame/ForceAdjacentSame 遇空值不比较
 *   - ForbidPair/ForcePair: 人员对相邻约束
 */

#include "ShuffleEngine.h"
#include <QRandomGenerator>
#include <algorithm>
#include <numeric>
#include "../ui/common/Common.h"

namespace seat {

    static constexpr int MAX_ATTEMPTS = 500000;

    // ============================================================
    // 辅助: 两座位是否同行相邻
    // ============================================================

    static bool areSameRowAdjacent(int seatIdxA, int seatIdxB, const QVector<ShuffleEngine::SeatPos> &seats,
                                   const Classroom &room) {
        if (seats[seatIdxA].row != seats[seatIdxB].row)
            return false;
        int colA = seats[seatIdxA].col, colB = seats[seatIdxB].col;
        if (colA > colB)
            std::swap(colA, colB);
        int row = seats[seatIdxA].row;
        for (int c = colA + 1; c < colB; ++c)
            if (room.grid[row][c] == CellType::Seat)
                return false;
        return true;
    }

    static int findSeatIndex(const QVector<ShuffleEngine::SeatPos> &seats, int row, int col) {
        for (int i = 0; i < seats.size(); ++i)
            if (seats[i].row == row && seats[i].col == col)
                return i;
        return -1;
    }

    // ============================================================
    // 主入口
    // ============================================================

    ShuffleEngine::Result ShuffleEngine::execute(const PersonGroup &group, const Classroom &room, const Rule &rule,
                                                 const QString &nameField) {
        Result result;
        result.status = ResultStatus::Success;

        if (group.persons.isEmpty()) {
            result.status = ResultStatus::InvalidInput;
            result.errorMessage = "人员组为空，无法抽取。";
            return result;
        }

        QVector<SeatPos> seats;
        for (int r = 0; r < room.rows; ++r)
            for (int c = 0; c < room.cols; ++c)
                if (room.grid[r][c] == CellType::Seat)
                    seats.append({r, c});

        if (seats.isEmpty()) {
            result.status = ResultStatus::InvalidInput;
            result.errorMessage = "教室中没有座位。";
            return result;
        }
        if (group.persons.size() > seats.size()) {
            result.status = ResultStatus::TooManyPersons;
            result.errorMessage =
                    QString("人数 (%1) 超过可用座位数 (%2)。").arg(group.persons.size()).arg(seats.size());
            return result;
        }

        // ─── 预分配: 尝试确定必须坐特定座位的人员 ──
        QVector<Person> allPersons = group.persons;
        QVector<int> seatAssignment(allPersons.size(), -1);
        QVector<bool> seatUsed(seats.size(), false);
        QSet<int> preAssigned;

        for (const auto &c: rule.constraints) {
            if (c.type != ConstraintType::SeatFieldMatch)
                continue;
            int tRow = c.params["row"].toInt();
            int tCol = c.params["col"].toInt();
            QString field = c.params["field"].toString();
            QString value = c.params["value"].toString();

            // 查找该字段+值唯一匹配的人员
            QVector<int> candidates;
            for (int i = 0; i < allPersons.size(); ++i) {
                if (preAssigned.contains(i))
                    continue;
                if (allPersons[i].fields.value(field) == value)
                    candidates.append(i);
            }
            // 唯一匹配 → 预分配
            if (candidates.size() == 1) {
                int pi = candidates[0];
                int si = findSeatIndex(seats, tRow, tCol);
                if (si >= 0 && !seatUsed[si]) {
                    seatAssignment[pi] = si;
                    seatUsed[si] = true;
                    preAssigned.insert(pi);
                }
            }
        }

        // ─── 构建待回溯人员列表并随机打乱 ──────────
        QVector<int> pending;
        for (int i = 0; i < allPersons.size(); ++i)
            if (!preAssigned.contains(i))
                pending.append(i);

        auto *rng = QRandomGenerator::global();
        for (int i = pending.size() - 1; i > 0; --i) {
            int j = rng->bounded(i + 1);
            std::swap(pending[i], pending[j]);
        }

        // ─── 回溯求解 ──────────────────────────────
        int attempts = 0;
        bool found = false;

        if (rule.constraints.isEmpty()) {
            QVector<int> freeSeatIdx;
            for (int i = 0; i < seats.size(); ++i)
                if (!seatUsed[i])
                    freeSeatIdx.append(i);
            for (int i = freeSeatIdx.size() - 1; i > 0; --i) {
                int j = rng->bounded(i + 1);
                std::swap(freeSeatIdx[i], freeSeatIdx[j]);
            }
            for (int i = 0; i < pending.size(); ++i)
                seatAssignment[pending[i]] = freeSeatIdx[i];
            found = true;
        } else {
            found = backtrack(0, pending, seatAssignment, seatUsed, allPersons, seats, room, rule.constraints, attempts,
                              MAX_ATTEMPTS);
        }

        if (!found) {
            result.status = ResultStatus::NoSolution;
            result.errorMessage = QString("在 %1 次尝试后未找到满足所有约束的分配方案。\n"
                                          "请检查规则约束是否过于严格或存在矛盾。\n"
                                          "(已预分配 %2 人)")
                                          .arg(attempts)
                                          .arg(preAssigned.size());
            return result;
        }

        // ─── 构建结果记录 ──────────────────────────
        ShuffleRecord &rec = result.record;
        rec.id = generateId();
        rec.timestamp = currentTimestamp();
        rec.groupId = group.id;
        rec.groupName = group.name;
        rec.classroomId = room.id;
        rec.classroomName = room.name;
        rec.ruleId = rule.id;
        rec.ruleName = rule.name;

        for (int p = 0; p < allPersons.size(); ++p) {
            int s = seatAssignment[p];
            if (s < 0)
                continue;
            SeatAssignment sa;
            sa.row = seats[s].row;
            sa.col = seats[s].col;
            sa.personId = allPersons[p].id;
            sa.personName =
                    nameField.isEmpty() ? allPersons[p].id : allPersons[p].fields.value(nameField, allPersons[p].id);
            rec.assignments.append(sa);
        }
        return result;
    }

    // ============================================================
    // 约束检查
    // ============================================================

    bool ShuffleEngine::checkConstraints(const QVector<int> &seatAssignment, const QVector<Person> &persons,
                                         const QVector<SeatPos> &seats, const Classroom &room,
                                         const QVector<Constraint> &constraints, bool partial) {
        QVector<int> s2p(seats.size(), -1);
        for (int p = 0; p < seatAssignment.size(); ++p)
            if (seatAssignment[p] >= 0)
                s2p[seatAssignment[p]] = p;

        for (const auto &c: constraints) {
            const auto &pm = c.params;
            switch (c.type) {

                case ConstraintType::SeatFieldMatch: {
                    int tRow = pm["row"].toInt(), tCol = pm["col"].toInt();
                    QString field = pm["field"].toString(), value = pm["value"].toString();
                    for (int si = 0; si < seats.size(); ++si) {
                        if (seats[si].row == tRow && seats[si].col == tCol) {
                            int pi = s2p[si];
                            if (pi >= 0 && persons[pi].fields.value(field) != value)
                                return false;
                            break;
                        }
                    }
                    break;
                }

                case ConstraintType::RowPattern: {
                    int tRow = pm["row"].toInt();
                    QString field = pm["field"].toString();
                    QJsonArray pattern = pm["pattern"].toArray();
                    QVector<int> rsi;
                    for (int si = 0; si < seats.size(); ++si)
                        if (seats[si].row == tRow)
                            rsi.append(si);
                    std::ranges::sort(rsi, [&](int a, int b) { return seats[a].col < seats[b].col; });
                    int idx = 0;
                    for (int pi = 0; pi < pattern.size() && idx < rsi.size(); ++pi) {
                        QString pat = pattern[pi].toString();
                        if (pat == "_")
                            continue;
                        int personIdx = s2p[rsi[idx]];
                        if (personIdx >= 0 && persons[personIdx].fields.value(field) != pat)
                            return false;
                        ++idx;
                    }
                    break;
                }

                case ConstraintType::NoAdjacentSame: {
                    QString field = pm["field"].toString();
                    QMap<int, QVector<int>> rowSeats;
                    for (int si = 0; si < seats.size(); ++si)
                        rowSeats[seats[si].row].append(si);
                    for (auto it = rowSeats.begin(); it != rowSeats.end(); ++it) {
                        auto &sl = it.value();
                        std::ranges::sort(sl, [&](int a, int b) { return seats[a].col < seats[b].col; });
                        for (int i = 1; i < sl.size(); ++i) {
                            int cA = seats[sl[i - 1]].col, cB = seats[sl[i]].col;
                            bool adj = true;
                            for (int col = cA + 1; col < cB; ++col)
                                if (room.grid[it.key()][col] == CellType::Seat) {
                                    adj = false;
                                    break;
                                }
                            if (!adj)
                                continue;
                            int pA = s2p[sl[i - 1]], pB = s2p[sl[i]];
                            if (pA >= 0 && pB >= 0) {
                                QString vA = persons[pA].fields.value(field);
                                QString vB = persons[pB].fields.value(field);
                                if (vA.isEmpty() || vB.isEmpty())
                                    continue; // 空值跳过
                                if (vA == vB)
                                    return false;
                            }
                        }
                    }
                    break;
                }

                case ConstraintType::ForceAdjacentSame: {
                    QString field = pm["field"].toString();
                    QMap<int, QVector<int>> rowSeats;
                    for (int si = 0; si < seats.size(); ++si)
                        rowSeats[seats[si].row].append(si);
                    for (auto it = rowSeats.begin(); it != rowSeats.end(); ++it) {
                        auto &sl = it.value();
                        std::ranges::sort(sl, [&](int a, int b) { return seats[a].col < seats[b].col; });
                        for (int i = 1; i < sl.size(); ++i) {
                            int cA = seats[sl[i - 1]].col, cB = seats[sl[i]].col;
                            bool adj = true;
                            for (int col = cA + 1; col < cB; ++col)
                                if (room.grid[it.key()][col] == CellType::Seat) {
                                    adj = false;
                                    break;
                                }
                            if (!adj)
                                continue;
                            int pA = s2p[sl[i - 1]], pB = s2p[sl[i]];
                            if (pA >= 0 && pB >= 0) {
                                QString vA = persons[pA].fields.value(field);
                                QString vB = persons[pB].fields.value(field);
                                if (vA.isEmpty() || vB.isEmpty())
                                    continue;
                                if (vA != vB)
                                    return false;
                            }
                        }
                    }
                    break;
                }

                case ConstraintType::ColumnFieldMatch: {
                    int tCol = pm["col"].toInt();
                    QString field = pm["field"].toString(), value = pm["value"].toString();
                    for (int si = 0; si < seats.size(); ++si)
                        if (seats[si].col == tCol) {
                            int pi = s2p[si];
                            if (pi >= 0 && persons[pi].fields.value(field) != value)
                                return false;
                        }
                    break;
                }

                case ConstraintType::ZoneFieldMatch: {
                    int rS = pm["rowStart"].toInt(), rE = pm["rowEnd"].toInt();
                    int cS = pm["colStart"].toInt(), cE = pm["colEnd"].toInt();
                    QString field = pm["field"].toString(), value = pm["value"].toString();
                    for (int si = 0; si < seats.size(); ++si) {
                        int sr = seats[si].row, sc = seats[si].col;
                        if (sr >= rS && sr <= rE && sc >= cS && sc <= cE) {
                            int pi = s2p[si];
                            if (pi >= 0 && persons[pi].fields.value(field) != value)
                                return false;
                        }
                    }
                    break;
                }

                case ConstraintType::ForbidPair: {
                    QString pid1 = pm["personId1"].toString(), pid2 = pm["personId2"].toString();
                    int i1 = -1, i2 = -1;
                    for (int i = 0; i < persons.size(); ++i) {
                        if (persons[i].id == pid1)
                            i1 = i;
                        if (persons[i].id == pid2)
                            i2 = i;
                    }
                    if (i1 < 0 || i2 < 0)
                        break;
                    int s1 = seatAssignment[i1], s2 = seatAssignment[i2];
                    if (s1 < 0 || s2 < 0)
                        break;
                    if (areSameRowAdjacent(s1, s2, seats, room))
                        return false;
                    break;
                }

                case ConstraintType::ForcePair: {
                    QString pid1 = pm["personId1"].toString(), pid2 = pm["personId2"].toString();
                    int i1 = -1, i2 = -1;
                    for (int i = 0; i < persons.size(); ++i) {
                        if (persons[i].id == pid1)
                            i1 = i;
                        if (persons[i].id == pid2)
                            i2 = i;
                    }
                    if (i1 < 0 || i2 < 0)
                        break;
                    int s1 = seatAssignment[i1], s2 = seatAssignment[i2];
                    if (s1 < 0 || s2 < 0)
                        break;
                    if (!areSameRowAdjacent(s1, s2, seats, room))
                        return false;
                    break;
                }

            } // switch
        } // for
        return true;
    }

    // ============================================================
    // 回溯搜索
    // ============================================================

    bool ShuffleEngine::backtrack(int step, const QVector<int> &pendingPersons, QVector<int> &seatAssignment,
                                  QVector<bool> &seatUsed, const QVector<Person> &persons,
                                  const QVector<SeatPos> &seats, const Classroom &room,
                                  const QVector<Constraint> &constraints, int &attempts, int maxAttempts) {
        if (step >= pendingPersons.size())
            return checkConstraints(seatAssignment, persons, seats, room, constraints, false);
        if (attempts >= maxAttempts)
            return false;

        int personIdx = pendingPersons[step];

        // 收集可用座位并随机打乱
        QVector<int> seatOrder;
        for (int i = 0; i < seats.size(); ++i)
            if (!seatUsed[i])
                seatOrder.append(i);
        auto *rng = QRandomGenerator::global();
        for (int i = seatOrder.size() - 1; i > 0; --i) {
            int j = rng->bounded(i + 1);
            std::swap(seatOrder[i], seatOrder[j]);
        }

        for (int si: seatOrder) {
            ++attempts;
            if (attempts >= maxAttempts)
                return false;

            seatAssignment[personIdx] = si;
            seatUsed[si] = true;

            if (checkConstraints(seatAssignment, persons, seats, room, constraints, true)) {
                if (backtrack(step + 1, pendingPersons, seatAssignment, seatUsed, persons, seats, room, constraints,
                              attempts, maxAttempts))
                    return true;
            }

            seatAssignment[personIdx] = -1;
            seatUsed[si] = false;
        }
        return false;
    }

} // namespace seat
