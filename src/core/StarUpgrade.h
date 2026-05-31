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
    // returnedItems：被合并消耗的单位所穿戴装备会追加到此列表（供 GUI 归还装备栏）。
    static bool tryMergeAll(std::vector<Unit*>& playerUnits, Board& board,
                             std::map<int, Unit*>& unitsMap, Player& player,
                             std::vector<ItemType>* returnedItems = nullptr);

private:
    static void removeUnit(int unitId, std::vector<Unit*>& playerUnits,
                           Board& board, std::map<int, Unit*>& unitsMap, Player& player,
                           std::vector<ItemType>* returnedItems = nullptr);
};

}  // namespace core
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_CORE_STAR_UPGRADE_H
