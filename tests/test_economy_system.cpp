#include "core/EconomySystem.h"

#include <gtest/gtest.h>
#include <stdexcept>

#include "core/Player.h"

using my_auto_arena::core::EconomyBreakdown;
using my_auto_arena::core::EconomySystem;
using my_auto_arena::core::Player;

TEST(EconomySystemTest, InterestGivesOneGoldPerTenGold) {
    EXPECT_EQ(EconomySystem::calculateInterestGold(0), 0);
    EXPECT_EQ(EconomySystem::calculateInterestGold(9), 0);
    EXPECT_EQ(EconomySystem::calculateInterestGold(10), 1);
    EXPECT_EQ(EconomySystem::calculateInterestGold(37), 3);
}

TEST(EconomySystemTest, WinStreakBonusUsesExpectedThresholds) {
    EXPECT_EQ(EconomySystem::calculateWinStreakBonus(1), 0);
    EXPECT_EQ(EconomySystem::calculateWinStreakBonus(2), 1);
    EXPECT_EQ(EconomySystem::calculateWinStreakBonus(3), 2);
    EXPECT_EQ(EconomySystem::calculateWinStreakBonus(4), 3);
    EXPECT_EQ(EconomySystem::calculateWinStreakBonus(8), 3);
}

TEST(EconomySystemTest, WinningRoundAddsBaseInterestAndStreakBonus) {
    Player player(1, 20, 100, 1, 3);
    player.setWinStreak(1);

    const EconomyBreakdown economy = EconomySystem::settleRound(player, true, 5);

    EXPECT_EQ(economy.baseGold, 5);
    EXPECT_EQ(economy.interestGold, 2);
    EXPECT_EQ(economy.winStreakBonus, 1);
    EXPECT_EQ(economy.totalGold, 8);
    EXPECT_EQ(economy.winStreak, 2);
    EXPECT_EQ(player.gold(), 28);
    EXPECT_EQ(player.winStreak(), 2);
}

TEST(EconomySystemTest, LosingRoundKeepsInterestAndResetsWinStreak) {
    Player player(1, 30, 100, 1, 3);
    player.setWinStreak(3);

    const EconomyBreakdown economy = EconomySystem::settleRound(player, false, 7);

    EXPECT_EQ(economy.baseGold, 0);
    EXPECT_EQ(economy.interestGold, 3);
    EXPECT_EQ(economy.winStreakBonus, 0);
    EXPECT_EQ(economy.totalGold, 3);
    EXPECT_EQ(economy.winStreak, 0);
    EXPECT_EQ(player.gold(), 33);
    EXPECT_EQ(player.winStreak(), 0);
}

TEST(EconomySystemTest, RejectsInvalidValues) {
    EXPECT_THROW(EconomySystem::calculateInterestGold(-1), std::invalid_argument);
    EXPECT_THROW(EconomySystem::calculateWinStreakBonus(-1), std::invalid_argument);

    Player player(1, 10, 100, 1, 3);
    EXPECT_THROW(EconomySystem::settleRound(player, true, -1), std::invalid_argument);
}
