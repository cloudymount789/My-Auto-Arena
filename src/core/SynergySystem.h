#ifndef MY_AUTO_ARENA_CORE_SYNERGY_SYSTEM_H
#define MY_AUTO_ARENA_CORE_SYNERGY_SYSTEM_H

#include <map>
#include <string>
#include <vector>

#include "core/Board.h"
#include "core/Unit.h"

namespace my_auto_arena {
namespace core {

// 已激活的羁绊信息，用于 UI 展示。
struct ActiveSynergy {
    std::string name;
    int count;
    int activeThreshold;   // 0 表示未激活
    std::string buffDescription;
};

// 羁绊系统：根据棋盘上的英雄职业组合计算并施加/清除羁绊 BUFF。
class SynergySystem {
public:
    // 根据棋盘上的单位职业分布计算并施加羁绊加成。
    static void applyBuffs(const Board& board, std::map<int, Unit*>& units);

    // 清除所有玩家单位的羁绊加成（每轮结算后调用）。
    static void clearBuffs(std::vector<Unit*>& playerUnits);

    // 返回当前所有羁绊的激活状态列表（供 UI 展示）。
    static std::vector<ActiveSynergy> getActiveSynergies(const Board& board,
                                                          const std::map<int, Unit*>& units);

private:
    // 统计棋盘上指定职业的玩家单位数量。
    static int countClassOnBoard(UnitClass cls, const Board& board,
                                 const std::map<int, Unit*>& units);
};

}  // namespace core
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_CORE_SYNERGY_SYSTEM_H
