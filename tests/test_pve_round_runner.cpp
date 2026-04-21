#include "core/PvERoundRunner.h"

#include <gtest/gtest.h>

#include <map>

#include "core/Board.h"
#include "core/EnemySpawner.h"
#include "core/Player.h"
#include "core/Unit.h"

using my_auto_arena::core::Board;
using my_auto_arena::core::EnemySpawner;
using my_auto_arena::core::Player;
using my_auto_arena::core::Position;
using my_auto_arena::core::PvERoundRunner;
using my_auto_arena::core::RoundOutcome;
using my_auto_arena::core::UnitOwner;
using my_auto_arena::core::WarriorUnit;

TEST(PvERoundRunnerTest, Round1PlayerBeatsSingleEnemy) {
    Board board(8, 8, 8);
    Player player(1, 10, 100, 1, 8);
    WarriorUnit ally(50, UnitOwner::player);
    std::map<int, my_auto_arena::core::Unit*> units;
    units[ally.id()] = &ally;
    player.addUnit(ally.id());
    ASSERT_TRUE(board.placeOnBoard(ally.id(), Position{7, 3}));

    EnemySpawner spawner;
    int nextId = 200;
    const RoundOutcome outcome = PvERoundRunner::runRoundBattle(board, player, units, 1, spawner, nextId);

    EXPECT_TRUE(outcome.playerWon);
    EXPECT_EQ(player.gold(), 15);
    EXPECT_EQ(units.size(), static_cast<std::size_t>(1));
    EXPECT_EQ(units.count(ally.id()), static_cast<std::size_t>(1));
}
