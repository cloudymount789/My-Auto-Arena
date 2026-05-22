#include "core/EnemySpawner.h"

#include <gtest/gtest.h>

#include <stdexcept>
#include <vector>

#include "core/Board.h"
#include "core/Unit.h"

using my_auto_arena::core::Board;
using my_auto_arena::core::EnemySpawner;
using my_auto_arena::core::LevelConfig;
using my_auto_arena::core::Unit;
using my_auto_arena::core::UnitOwner;

TEST(EnemySpawnerTest, Round1SpawnsTwoEnemiesOnBoard) {
    Board board(8, 8, 8);
    EnemySpawner spawner;
    int nextId = 100;

    std::vector<Unit*> spawned = spawner.spawnRound(1, board, nextId);
    ASSERT_EQ(spawned.size(), 2);
    EXPECT_EQ(spawned.at(0)->owner(), UnitOwner::enemy);
    EXPECT_EQ(spawned.at(0)->name(), "战士");
    EXPECT_EQ(spawned.at(0)->hp(), 1700);
    EXPECT_EQ(spawned.at(0)->attack(), 68);

    const my_auto_arena::core::Position pos = board.findUnitOnBoard(spawned.at(0)->id());
    EXPECT_TRUE(board.inBounds(pos));

    for (std::size_t i = 0; i < spawned.size(); ++i) {
        delete spawned.at(i);
    }
}

// 无尽关卡模式：第7关及以上不再抛出异常，而是按指数公式生成敌方配置。
TEST(EnemySpawnerTest, InvalidRoundThrowsOutOfRange) {
    Board board(8, 8, 8);
    EnemySpawner spawner;
    int nextId = 100;
    // round=7 是无尽关的第一关，应正常产出敌方单位，不抛出异常。
    EXPECT_NO_THROW(spawner.spawnRound(7, board, nextId));
    // round=0 应被 clamp 为 round=1，同样不抛出。
    int nextId2 = 200;
    EXPECT_NO_THROW(spawner.spawnRound(0, board, nextId2));
}

TEST(EnemySpawnerTest, Round2SpawnsThreeEnemies) {
    Board board(8, 8, 8);
    EnemySpawner spawner;
    int nextId = 200;

    std::vector<Unit*> spawned = spawner.spawnRound(2, board, nextId);
    ASSERT_EQ(spawned.size(), 3);

    for (std::size_t i = 0; i < spawned.size(); ++i) {
        EXPECT_EQ(spawned.at(i)->owner(), UnitOwner::enemy);
        delete spawned.at(i);
    }
}
