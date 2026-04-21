#include "core/BattleEngine.h"

#include <gtest/gtest.h>

#include <map>

#include "core/Board.h"
#include "core/Unit.h"

using my_auto_arena::core::BattleEngine;
using my_auto_arena::core::Board;
using my_auto_arena::core::MageUnit;
using my_auto_arena::core::Position;
using my_auto_arena::core::RoundOutcome;
using my_auto_arena::core::Unit;
using my_auto_arena::core::UnitOwner;
using my_auto_arena::core::WarriorUnit;

TEST(BattleEngineTest, MeleeKillsEnemyInRangeAndEndsBattle) {
    Board board(8, 8, 8);
    WarriorUnit player(1, UnitOwner::player);
    WarriorUnit enemy(2, UnitOwner::enemy);
    ASSERT_TRUE(board.placeOnBoard(player.id(), Position{6, 3}));
    ASSERT_TRUE(board.placeOnBoard(enemy.id(), Position{6, 4}));

    std::map<int, Unit*> units;
    units[player.id()] = &player;
    units[enemy.id()] = &enemy;
    BattleEngine engine(board, units);

    for (int i = 0; i < 20 && !engine.isFinished(); ++i) {
        engine.tick();
    }
    EXPECT_TRUE(engine.isFinished());
    EXPECT_TRUE(engine.outcome().playerWon);
    EXPECT_EQ(board.occupantOnBoard(Position{6, 4}), Board::kEmptySlot);
}

TEST(BattleEngineTest, OutOfRangeDoesNotDealDamageOnFirstTick) {
    Board board(8, 8, 8);
    WarriorUnit player(1, UnitOwner::player);
    WarriorUnit enemy(2, UnitOwner::enemy);
    ASSERT_TRUE(board.placeOnBoard(player.id(), Position{7, 0}));
    ASSERT_TRUE(board.placeOnBoard(enemy.id(), Position{0, 7}));

    std::map<int, Unit*> units;
    units[player.id()] = &player;
    units[enemy.id()] = &enemy;
    BattleEngine engine(board, units);

    engine.tick();
    EXPECT_EQ(player.hp(), player.maxHp());
    EXPECT_EQ(enemy.hp(), enemy.maxHp());
}

TEST(BattleEngineTest, FarMeleeEventuallyEngagesAndFinishes) {
    Board board(8, 8, 8);
    WarriorUnit player(1, UnitOwner::player);
    WarriorUnit enemy(2, UnitOwner::enemy);
    ASSERT_TRUE(board.placeOnBoard(player.id(), Position{7, 0}));
    ASSERT_TRUE(board.placeOnBoard(enemy.id(), Position{0, 7}));

    std::map<int, Unit*> units;
    units[player.id()] = &player;
    units[enemy.id()] = &enemy;
    BattleEngine engine(board, units);

    for (int i = 0; i < 2000 && !engine.isFinished(); ++i) {
        engine.tick();
    }
    EXPECT_TRUE(engine.isFinished());
    EXPECT_TRUE(engine.outcome().playerWon);
}

TEST(BattleEngineTest, AttackAddsManaToAttacker) {
    Board board(8, 8, 8);
    MageUnit player(1, UnitOwner::player);
    WarriorUnit enemy(2, UnitOwner::enemy);
    ASSERT_TRUE(board.placeOnBoard(player.id(), Position{6, 1}));
    ASSERT_TRUE(board.placeOnBoard(enemy.id(), Position{6, 3}));

    std::map<int, Unit*> units;
    units[player.id()] = &player;
    units[enemy.id()] = &enemy;
    BattleEngine engine(board, units);

    engine.tick();
    EXPECT_EQ(player.mana(), BattleEngine::kManaPerAttack);
}

TEST(BattleEngineTest, StopsWithinMaxTickBudget) {
    Board board(8, 8, 8);
    WarriorUnit player(1, UnitOwner::player);
    WarriorUnit enemy(2, UnitOwner::enemy);
    ASSERT_TRUE(board.placeOnBoard(player.id(), Position{7, 0}));
    ASSERT_TRUE(board.placeOnBoard(enemy.id(), Position{0, 7}));

    std::map<int, Unit*> units;
    units[player.id()] = &player;
    units[enemy.id()] = &enemy;
    BattleEngine engine(board, units);

    for (int i = 0; i < BattleEngine::kMaxTicks; ++i) {
        engine.tick();
        if (engine.isFinished()) {
            break;
        }
    }
    EXPECT_TRUE(engine.isFinished());
    EXPECT_LE(engine.tickCount(), BattleEngine::kMaxTicks);
}
