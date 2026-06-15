#include "core/EconomySystem.h"

#include <stdexcept>

namespace my_auto_arena {
namespace core {

// 流程：校验金币非负 ──> 每 10 金币转换为 1 金币利息 ──> 返回利息值
int EconomySystem::calculateInterestGold(int currentGold) {
    if (currentGold < 0) {
        throw std::invalid_argument("Current gold cannot be negative.");
    }
    return currentGold / 10;
}

// 流程：读取当前连胜 ──> 按 2/3/4+ 连胜给 1/2/3 金币 ──> 返回奖励
int EconomySystem::calculateWinStreakBonus(int winStreak) {
    if (winStreak < 0) {
        throw std::invalid_argument("Win streak cannot be negative.");
    }
    if (winStreak >= 4) {
        return 3;
    }
    if (winStreak == 3) {
        return 2;
    }
    if (winStreak == 2) {
        return 1;
    }
    return 0;
}

// 流程：按胜负更新连胜 ──> 基于结算前金币计算利息 ──> 加总奖励并写回玩家金币
EconomyBreakdown EconomySystem::settleRound(Player& player, bool playerWon, int baseGold) {
    if (baseGold < 0) {
        throw std::invalid_argument("Base gold cannot be negative.");
    }

    EconomyBreakdown breakdown;
    breakdown.baseGold = playerWon ? baseGold : 0;
    breakdown.interestGold = calculateInterestGold(player.gold());

    if (playerWon) {
        player.setWinStreak(player.winStreak() + 1);
    } else {
        player.setWinStreak(0);
    }
    breakdown.winStreak = player.winStreak();
    breakdown.winStreakBonus = playerWon ? calculateWinStreakBonus(breakdown.winStreak) : 0;
    breakdown.totalGold = breakdown.baseGold + breakdown.interestGold + breakdown.winStreakBonus;
    player.setGold(player.gold() + breakdown.totalGold);
    return breakdown;
}

}  // namespace core
}  // namespace my_auto_arena
