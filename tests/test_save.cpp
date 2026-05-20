#include <gtest/gtest.h>

#include <cstdio>
#include <map>
#include <vector>

#include "core/Board.h"
#include "core/GameFSM.h"
#include "core/HeroUnits.h"
#include "core/Item.h"
#include "core/Player.h"
#include "core/SaveManager.h"
#include "core/Unit.h"

using namespace my_auto_arena::core;

// ── 辅助：生成临时文件路径 ────────────────────────────────────────────────────

static std::string tempFilePath() {
    return "test_save_temp.txt";
}

// ── 存读档基本流程测试 ────────────────────────────────────────────────────────

TEST(SaveTest, SaveAndLoadRoundTrip) {
    // 准备原始状态
    GameFSM fsm;
    Player player(1, 15, 80, 1, 4);
    Board board(8, 8, 8);

    std::vector<Unit*> playerUnits;
    std::map<int, Unit*> unitsMap;

    Unit* hero1 = new AshRaiderHero(1, UnitOwner::player);
    Unit* hero2 = new NightArcherHero(2, UnitOwner::player);
    hero1->equipItem(ItemType::kSword);

    playerUnits.push_back(hero1);
    playerUnits.push_back(hero2);
    unitsMap[hero1->id()] = hero1;
    unitsMap[hero2->id()] = hero2;
    player.addUnit(hero1->id());
    player.addUnit(hero2->id());

    board.placeOnBoard(hero1->id(), Position{6, 0});
    board.placeOnBench(hero2->id(), 0);

    std::vector<ItemType> pendingItems = {ItemType::kArmor};

    const std::string filepath = tempFilePath();

    // 存档
    const bool saved = SaveManager::save(filepath, fsm, player, board, playerUnits, pendingItems);
    ASSERT_TRUE(saved);

    // 准备读档目标（清空状态）
    GameFSM fsmLoad;
    Player playerLoad(1, 0, 100, 1, 3);
    Board boardLoad(8, 8, 8);
    std::vector<Unit*> playerUnitsLoad;
    std::map<int, Unit*> unitsMapLoad;
    std::vector<ItemType> pendingLoad;

    const bool loaded = SaveManager::load(filepath, fsmLoad, playerLoad, boardLoad,
                                          playerUnitsLoad, unitsMapLoad, pendingLoad);
    ASSERT_TRUE(loaded);

    // 验证恢复的玩家状态
    EXPECT_EQ(playerLoad.hp(), 80);
    EXPECT_EQ(playerLoad.gold(), 15);
    EXPECT_EQ(playerLoad.populationCap(), 4);

    // 验证单位数量
    EXPECT_EQ(playerUnitsLoad.size(), static_cast<std::size_t>(2));

    // 验证待装备道具
    ASSERT_EQ(pendingLoad.size(), static_cast<std::size_t>(1));
    EXPECT_EQ(pendingLoad.at(0), ItemType::kArmor);

    // 验证英雄1的装备被还原
    bool foundWithSword = false;
    for (std::size_t i = 0; i < playerUnitsLoad.size(); ++i) {
        if (playerUnitsLoad.at(i)->equippedItem() == ItemType::kSword) {
            foundWithSword = true;
        }
    }
    EXPECT_TRUE(foundWithSword);

    // 验证棋盘位置还原（hero1 在棋盘上）
    bool hero1OnBoard = false;
    for (int row = 0; row < boardLoad.rows(); ++row) {
        for (int col = 0; col < boardLoad.cols(); ++col) {
            if (boardLoad.occupantOnBoard(Position{row, col}) != Board::kEmptySlot) {
                hero1OnBoard = true;
            }
        }
    }
    EXPECT_TRUE(hero1OnBoard);

    // 清理
    for (std::size_t i = 0; i < playerUnitsLoad.size(); ++i) {
        delete playerUnitsLoad.at(i);
    }
    for (std::size_t i = 0; i < playerUnits.size(); ++i) {
        delete playerUnits.at(i);
    }
    std::remove(filepath.c_str());
}

TEST(SaveTest, SaveToInvalidPathReturnsFalse) {
    GameFSM fsm;
    Player player(1, 10, 100, 1, 3);
    Board board(8, 8, 8);
    std::vector<Unit*> playerUnits;
    std::vector<ItemType> pendingItems;

    const bool ok = SaveManager::save("/nonexistent_dir/save.txt", fsm, player, board,
                                      playerUnits, pendingItems);
    EXPECT_FALSE(ok);
}

TEST(SaveTest, LoadFromNonexistentFileReturnsFalse) {
    GameFSM fsm;
    Player player(1, 10, 100, 1, 3);
    Board board(8, 8, 8);
    std::vector<Unit*> playerUnits;
    std::map<int, Unit*> unitsMap;
    std::vector<ItemType> pendingItems;

    const bool ok = SaveManager::load("this_file_does_not_exist.txt", fsm, player, board,
                                      playerUnits, unitsMap, pendingItems);
    EXPECT_FALSE(ok);
}

// ── 星级存读档测试 ────────────────────────────────────────────────────────────

TEST(SaveTest, SaveAndLoadStarLevel) {
    GameFSM fsm;
    Player player(1, 10, 100, 1, 4);
    Board board(8, 8, 8);

    std::vector<Unit*> playerUnits;
    std::map<int, Unit*> unitsMap;

    Unit* hero = new AshRaiderHero(1, UnitOwner::player);
    hero->upgradeToStar(2);
    playerUnits.push_back(hero);
    unitsMap[hero->id()] = hero;
    player.addUnit(hero->id());
    board.placeOnBench(hero->id(), 0);

    std::vector<ItemType> pendingItems;
    const std::string filepath = "test_save_star.txt";

    ASSERT_TRUE(SaveManager::save(filepath, fsm, player, board, playerUnits, pendingItems));

    GameFSM fsmLoad;
    Player playerLoad(1, 0, 100, 1, 3);
    Board boardLoad(8, 8, 8);
    std::vector<Unit*> playerUnitsLoad;
    std::map<int, Unit*> unitsMapLoad;
    std::vector<ItemType> pendingLoad;

    ASSERT_TRUE(SaveManager::load(filepath, fsmLoad, playerLoad, boardLoad,
                                  playerUnitsLoad, unitsMapLoad, pendingLoad));

    ASSERT_EQ(playerUnitsLoad.size(), static_cast<std::size_t>(1));
    EXPECT_EQ(playerUnitsLoad.at(0)->starLevel(), 2);

    delete playerUnits.at(0);
    delete playerUnitsLoad.at(0);
    std::remove(filepath.c_str());
}
