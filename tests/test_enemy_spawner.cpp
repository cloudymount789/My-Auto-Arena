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

namespace {

int totalMaxHp(const std::vector<Unit*>& units) {
    int total = 0;
    for (std::size_t i = 0; i < units.size(); ++i) {
        total += units.at(i)->maxHp();
    }
    return total;
}

void deleteUnits(std::vector<Unit*>& units) {
    for (std::size_t i = 0; i < units.size(); ++i) {
        delete units.at(i);
    }
    units.clear();
}

}  // namespace

TEST(EnemySpawnerTest, Round1SpawnsTwoEnemiesOnBoard) {
    Board board(8, 8, 8);
    EnemySpawner spawner;
    int nextId = 100;

    std::vector<Unit*> spawned = spawner.spawnRound(1, board, nextId);
    ASSERT_EQ(spawned.size(), 2);
    EXPECT_EQ(spawned.at(0)->owner(), UnitOwner::enemy);
    EXPECT_EQ(spawned.at(0)->name(), "战士");
    EXPECT_EQ(spawned.at(0)->hp(), 689);
    EXPECT_EQ(spawned.at(0)->attack(), 29);

    const my_auto_arena::core::Position pos = board.findUnitOnBoard(spawned.at(0)->id());
    EXPECT_TRUE(board.inBounds(pos));

    deleteUnits(spawned);
}

// 无尽关卡模式：第7关及以上不再抛出异常，而是按指数公式生成敌方配置。
TEST(EnemySpawnerTest, InvalidRoundThrowsOutOfRange) {
    Board board(8, 8, 8);
    EnemySpawner spawner;
    int nextId = 100;
    // round=7 是无尽关的第一关，应正常产出敌方单位，不抛出异常。
    EXPECT_NO_THROW(spawner.spawnRound(7, board, nextId));
    // round=0 应被限制为 round=1，同样不抛出。
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
    }
    deleteUnits(spawned);
}

TEST(EnemySpawnerTest, EndlessRoundsIncreaseArmyHpBudgetSmoothly) {
    EnemySpawner spawner;
    int nextId = 300;

    Board board7(8, 8, 8);
    std::vector<Unit*> round7 = spawner.spawnRound(7, board7, nextId);
    const int hp7 = totalMaxHp(round7);
    deleteUnits(round7);

    Board board8(8, 8, 8);
    std::vector<Unit*> round8 = spawner.spawnRound(8, board8, nextId);
    const int hp8 = totalMaxHp(round8);
    deleteUnits(round8);

    Board board9(8, 8, 8);
    std::vector<Unit*> round9 = spawner.spawnRound(9, board9, nextId);
    const int hp9 = totalMaxHp(round9);
    deleteUnits(round9);

    Board board10(8, 8, 8);
    std::vector<Unit*> round10 = spawner.spawnRound(10, board10, nextId);
    const int hp10 = totalMaxHp(round10);
    deleteUnits(round10);

    EXPECT_GT(hp8, hp7);
    EXPECT_GT(hp9, hp8);
    EXPECT_GT(hp10, hp9);
    EXPECT_LT(hp8 - hp7, hp7 / 4);
    EXPECT_LT(hp9 - hp8, hp8 / 4);
    EXPECT_LT(hp10 - hp9, hp9 / 4);
}
