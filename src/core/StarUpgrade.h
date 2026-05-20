#ifndef MY_AUTO_ARENA_CORE_STAR_UPGRADE_H
#define MY_AUTO_ARENA_CORE_STAR_UPGRADE_H

#include <map>
#include <vector>

#include "core/Board.h"
#include "core/Player.h"
#include "core/Unit.h"

namespace my_auto_arena {
namespace core {

// 升星系统：当玩家拥有 3 个同名同星级英雄时自动合并为更高星级。
class StarUpgrade {
public:
    // 遍历玩家所有单位，发现 3 合 1 条件则执行合并，返回是否发生过合并。
    static bool tryMergeAll(std::vector<Unit*>& playerUnits, Board& board,
                             std::map<int, Unit*>& unitsMap, Player& player);

private:
    // 从玩家单位表、棋盘/备战区、全局单位表中移除指定单位并释放内存。
    static void removeUnit(int unitId, std::vector<Unit*>& playerUnits,
                           Board& board, std::map<int, Unit*>& unitsMap, Player& player);
};

}  // namespace core
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_CORE_STAR_UPGRADE_H
