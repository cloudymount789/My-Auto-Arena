#ifndef MY_AUTO_ARENA_CORE_PATHFINDER_H
#define MY_AUTO_ARENA_CORE_PATHFINDER_H

#include <map>

#include "core/Board.h"

namespace my_auto_arena {
namespace core {

class Unit;

// 基于 BFS 的网格寻路：用于战斗中向可攻击格移动一步（不实现斜走）。
class Pathfinder {
public:
    // 若存在从 start 出发、经空格移动一步可达的“可普攻格”（与 targetPos 欧氏距离 <= attackRange），
    // 则返回 true 并将 outNext 设为路径上的第一格（与 start 相邻）。
    static bool nextStepTowardAttackRange(const Board& board, const std::map<int, Unit*>& units, int movingUnitId,
                                          Position start, Position targetPos, int attackRange, Position* outNext);
};

}  // namespace core
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_CORE_PATHFINDER_H
