#include "core/EnemySpawner.h"

#include <gtest/gtest.h>

#include <vector>

#include "core/Board.h"
#include "core/Unit.h"

using my_auto_arena::core::Board;
using my_auto_arena::core::EnemySpawner;
using my_auto_arena::core::LevelConfig;
using my_auto_arena::core::Unit;
using my_auto_arena::core::UnitOwner;

TEST(EnemySpawnerTest, Round1SpawnsOneEnemyOnBoard) {
    Board board(8, 8, 8);
    EnemySpawner spawner;
    int nextId = 100;

    std::vector<Unit*> spawned = spawner.spawnRound(1, board, nextId);
    ASSERT_EQ(spawned.size(), 1);
    EXPECT_EQ(spawned.at(0)->owner(), UnitOwner::enemy);
    EXPECT_EQ(spawned.at(0)->name(), "灰烬掠袭者");
    EXPECT_EQ(spawned.at(0)->hp(), 680);
    EXPECT_EQ(spawned.at(0)->attack(), 70);

    const my_auto_arena::core::Position pos = board.findUnitOnBoard(spawned.at(0)->id());
    EXPECT_TRUE(board.inBounds(pos));

    for (std::size_t i = 0; i < spawned.size(); ++i) {
        delete spawned.at(i);
    }
}

TEST(EnemySpawnerTest, InvalidRoundSpawnsEmpty) {
    Board board(8, 8, 8);
    EnemySpawner spawner;
    int nextId = 100;
    std::vector<Unit*> spawned = spawner.spawnRound(99, board, nextId);
    EXPECT_TRUE(spawned.empty());
}

TEST(EnemySpawnerTest, Round2SpawnsTwoEnemies) {
    Board board(8, 8, 8);
    EnemySpawner spawner;
    int nextId = 200;

    std::vector<Unit*> spawned = spawner.spawnRound(2, board, nextId);
    ASSERT_EQ(spawned.size(), 2);

    for (std::size_t i = 0; i < spawned.size(); ++i) {
        EXPECT_EQ(spawned.at(i)->owner(), UnitOwner::enemy);
        delete spawned.at(i);
    }
}
