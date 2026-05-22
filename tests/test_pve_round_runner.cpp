#include "core/PvERoundRunner.h"

#include <gtest/gtest.h>

#include <map>

#include "core/Board.h"
#include "core/EnemySpawner.h"
#include "core/HeroUnits.h"
#include "core/Player.h"
#include "core/Unit.h"

using my_auto_arena::core::AshRaiderHero;
using my_auto_arena::core::Board;
using my_auto_arena::core::EnemySpawner;
using my_auto_arena::core::Player;
using my_auto_arena::core::Position;
using my_auto_arena::core::PvERoundRunner;
using my_auto_arena::core::RoundOutcome;
using my_auto_arena::core::UnitOwner;

// Round 1 now spawns 2 enemy warriors (HP=1700, ATK=68).
// 2 AshRaiders (HP=1600, ATK=62, burst skill 280) should win together.
TEST(PvERoundRunnerTest, Round1PlayerBeatsTwoEnemies) {
    Board board(8, 8, 8);
    Player player(1, 10, 100, 1, 8);
    AshRaiderHero ally1(50, UnitOwner::player);
    AshRaiderHero ally2(51, UnitOwner::player);
    std::map<int, my_auto_arena::core::Unit*> units;
    units[ally1.id()] = &ally1;
    units[ally2.id()] = &ally2;
    player.addUnit(ally1.id());
    player.addUnit(ally2.id());
    ASSERT_TRUE(board.placeOnBoard(ally1.id(), Position{7, 3}));
    ASSERT_TRUE(board.placeOnBoard(ally2.id(), Position{7, 4}));

    EnemySpawner spawner;
    int nextId = 200;
    const RoundOutcome outcome = PvERoundRunner::runRoundBattle(board, player, units, 1, spawner, nextId);

    EXPECT_TRUE(outcome.playerWon);
    // Round 1 win reward = 4 gold; starting gold = 10 → total 14.
    EXPECT_EQ(player.gold(), 14);
}
