#ifndef MY_AUTO_ARENA_CORE_ECONOMY_SYSTEM_H
#define MY_AUTO_ARENA_CORE_ECONOMY_SYSTEM_H

#include "core/Player.h"

namespace my_auto_arena {
namespace core {

struct EconomyBreakdown {
    int baseGold;
    int interestGold;
    int winStreakBonus;
    int totalGold;
    int winStreak;
};

// 高级经济系统：统一计算回合基础金币、存款利息与连胜奖励。
class EconomySystem {
public:
    static int calculateInterestGold(int currentGold);
    static int calculateWinStreakBonus(int winStreak);
    static EconomyBreakdown settleRound(Player& player, bool playerWon, int baseGold);
};

}  // namespace core
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_CORE_ECONOMY_SYSTEM_H
