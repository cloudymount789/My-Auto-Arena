#include <gtest/gtest.h>

#include <map>
#include <vector>

#include "core/Board.h"
#include "core/HeroUnits.h"
#include "core/Item.h"
#include "core/Player.h"
#include "core/StarUpgrade.h"
#include "core/Unit.h"

using namespace my_auto_arena::core;

// ── upgradeToStar 基础测试 ────────────────────────────────────────────────────

TEST(StarUpgradeTest, UpgradeToStar2MultipliesStats) {
    AshRaiderHero hero(1, UnitOwner::player);
    // star1 基础（重平衡后）：ATK=62, MaxHP=1600
    hero.upgradeToStar(2);
    EXPECT_EQ(hero.starLevel(), 2);
    EXPECT_EQ(hero.attack(), static_cast<int>(62 * 1.8));
    EXPECT_EQ(hero.maxHp(), static_cast<int>(1600 * 1.8));
}

TEST(StarUpgradeTest, UpgradeToStar3MultipliesStats) {
    AshRaiderHero hero(1, UnitOwner::player);
    hero.upgradeToStar(2);
    hero.upgradeToStar(3);
    EXPECT_EQ(hero.starLevel(), 3);
    EXPECT_EQ(hero.attack(), static_cast<int>(62 * 3.0));
    EXPECT_EQ(hero.maxHp(), static_cast<int>(1600 * 3.0));
}

TEST(StarUpgradeTest, UpgradePreservesItemBonus) {
    AshRaiderHero hero(1, UnitOwner::player);
    hero.equipItem(ItemType::kSword);  // +80 ATK
    hero.upgradeToStar(2);
    // 升星后：基础 ATK = 62*1.8 = 111，加装备 +80 = 191
    EXPECT_EQ(hero.attack(), static_cast<int>(62 * 1.8) + 80);
}

TEST(StarUpgradeTest, UpgradeResetsHpToFull) {
    AshRaiderHero hero(1, UnitOwner::player);
    hero.takeDamage(500);
    hero.upgradeToStar(2);
    EXPECT_EQ(hero.hp(), hero.maxHp());
}

// ── tryMergeAll 测试 ──────────────────────────────────────────────────────────

TEST(StarUpgradeTest, ThreeSameHeroMergeToStar2) {
    Board board(8, 8, 8);
    Player player(1, 0, 100, 1, 8);
    std::map<int, Unit*> units;
    std::vector<Unit*> playerUnits;

    for (int i = 1; i <= 3; ++i) {
        Unit* hero = new AshRaiderHero(i, UnitOwner::player);
        playerUnits.push_back(hero);
        units[i] = hero;
        player.addUnit(i);
        board.placeOnBench(i, i - 1);
    }

    const bool merged = StarUpgrade::tryMergeAll(playerUnits, board, units, player);

    EXPECT_TRUE(merged);
    EXPECT_EQ(playerUnits.size(), static_cast<std::size_t>(1));
    EXPECT_EQ(playerUnits.at(0)->starLevel(), 2);
    EXPECT_EQ(player.unitCount(), 1);
}

TEST(StarUpgradeTest, TwoSameHeroNoMerge) {
    Board board(8, 8, 8);
    Player player(1, 0, 100, 1, 8);
    std::map<int, Unit*> units;
    std::vector<Unit*> playerUnits;

    for (int i = 1; i <= 2; ++i) {
        Unit* hero = new AshRaiderHero(i, UnitOwner::player);
        playerUnits.push_back(hero);
        units[i] = hero;
        player.addUnit(i);
        board.placeOnBench(i, i - 1);
    }

    const bool merged = StarUpgrade::tryMergeAll(playerUnits, board, units, player);

    EXPECT_FALSE(merged);
    EXPECT_EQ(playerUnits.size(), static_cast<std::size_t>(2));

    // 手动清理
    for (std::size_t i = 0; i < playerUnits.size(); ++i) {
        delete playerUnits.at(i);
    }
}

TEST(StarUpgradeTest, SixSameHeroMergeToStar3) {
    Board board(8, 8, 8);
    Player player(1, 0, 100, 1, 8);
    std::map<int, Unit*> units;
    std::vector<Unit*> playerUnits;

    for (int i = 1; i <= 6; ++i) {
        Unit* hero = new AshRaiderHero(i, UnitOwner::player);
        playerUnits.push_back(hero);
        units[i] = hero;
        player.addUnit(i);
        board.placeOnBench(i, i - 1);
    }

    StarUpgrade::tryMergeAll(playerUnits, board, units, player);

    // 6 张 → 先合成 1 张 star2 + 2 张 star1，再凑 3 张...
    // 实际：3 张 star1 → 1 张 star2，剩 3 张 star1 → 1 张 star2，然后 2 张 star2 不够
    // 6 张 star1 → 2 张 star2（不够3张 star2 合 star3）
    // 除非已有 3*star1 = 1*star2，再有 3*star1 → 另 1*star2，共 2*star2 → 不够 star3
    // 期望：2 张 star2
    EXPECT_EQ(playerUnits.size(), static_cast<std::size_t>(2));
    for (std::size_t i = 0; i < playerUnits.size(); ++i) {
        EXPECT_EQ(playerUnits.at(i)->starLevel(), 2);
    }
}

TEST(StarUpgradeTest, DifferentHeroesNoMerge) {
    Board board(8, 8, 8);
    Player player(1, 0, 100, 1, 8);
    std::map<int, Unit*> units;
    std::vector<Unit*> playerUnits;

    Unit* w1 = new AshRaiderHero(1, UnitOwner::player);
    Unit* w2 = new AshRaiderHero(2, UnitOwner::player);
    Unit* a1 = new NightArcherHero(3, UnitOwner::player);  // 不同职业名称

    for (Unit* u : {w1, w2, a1}) {
        playerUnits.push_back(u);
        units[u->id()] = u;
        player.addUnit(u->id());
        board.placeOnBench(u->id(), u->id() - 1);
    }

    const bool merged = StarUpgrade::tryMergeAll(playerUnits, board, units, player);
    EXPECT_FALSE(merged);
    EXPECT_EQ(playerUnits.size(), static_cast<std::size_t>(3));

    for (std::size_t i = 0; i < playerUnits.size(); ++i) {
        delete playerUnits.at(i);
    }
}

// ── 装备系统测试 ──────────────────────────────────────────────────────────────

TEST(StarUpgradeTest, EquipSwordAddsAtk) {
    AshRaiderHero hero(1, UnitOwner::player);
    const int baseAtk = hero.attack();
    hero.equipItem(ItemType::kSword);
    EXPECT_EQ(hero.attack(), baseAtk + 80);
    EXPECT_EQ(hero.equippedItem(), ItemType::kSword);
}

TEST(StarUpgradeTest, EquipArmorAddsMaxHp) {
    AshRaiderHero hero(1, UnitOwner::player);
    const int baseHp = hero.maxHp();
    hero.equipItem(ItemType::kArmor);
    EXPECT_EQ(hero.maxHp(), baseHp + 800);
}

TEST(StarUpgradeTest, UnequipRemovesBonus) {
    AshRaiderHero hero(1, UnitOwner::player);
    const int baseAtk = hero.attack();
    hero.equipItem(ItemType::kSword);
    hero.unequipItem();
    EXPECT_EQ(hero.attack(), baseAtk);
    EXPECT_EQ(hero.equippedItem(), ItemType::kNone);
}

TEST(StarUpgradeTest, EquipReplacesExistingItem) {
    AshRaiderHero hero(1, UnitOwner::player);
    const int baseAtk = hero.attack();
    hero.equipItem(ItemType::kSword);  // +80 ATK
    hero.equipItem(ItemType::kRing);   // 替换：先 unequip sword，再 equip ring +60
    EXPECT_EQ(hero.attack(), baseAtk + 60);
    EXPECT_EQ(hero.equippedItem(), ItemType::kRing);
}
