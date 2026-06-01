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
    // 1 星基础（重平衡后）：物攻=62，最大生命=1600
    // ★2 倍率 = 3.0x
    hero.upgradeToStar(2);
    EXPECT_EQ(hero.starLevel(), 2);
    EXPECT_EQ(hero.attack(), static_cast<int>(62 * 3.0));
    EXPECT_EQ(hero.maxHp(), static_cast<int>(1600 * 3.0));
}

TEST(StarUpgradeTest, UpgradeToStar3MultipliesStats) {
    AshRaiderHero hero(1, UnitOwner::player);
    hero.upgradeToStar(2);
    hero.upgradeToStar(3);
    // ★3 倍率 = 7.0x（相对 1 星基础）
    EXPECT_EQ(hero.starLevel(), 3);
    EXPECT_EQ(hero.attack(), static_cast<int>(62 * 7.0));
    EXPECT_EQ(hero.maxHp(), static_cast<int>(1600 * 7.0));
}

TEST(StarUpgradeTest, UpgradePreservesItemBonus) {
    AshRaiderHero hero(1, UnitOwner::player);
    hero.equipItem(ItemType::kSword);  // +15% 物攻（基于基础值）
    hero.upgradeToStar(2);
    const int star2BaseAtk = static_cast<int>(62 * 3.0);
    const int swordBonus = static_cast<int>(star2BaseAtk * 0.15 + 0.5);
    EXPECT_EQ(hero.attack(), star2BaseAtk + swordBonus);
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

    // 6 张 → 先合成 1 张 2 星 + 2 张 1 星，再凑 3 张...
    // 实际：3 张 1 星 → 1 张 2 星，剩 3 张 1 星 → 1 张 2 星，然后 2 张 2 星不够
    // 6 张 1 星 → 2 张 2 星（不够 3 张 2 星合 3 星）
    // 除非已有 3×1 星 = 1×2 星，再有 3×1 星 → 另 1×2 星，共 2×2 星 → 不够 3 星
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
    const int baseAtk = hero.basePhysicalAtk();
    hero.equipItem(ItemType::kSword);
    const int swordBonus = static_cast<int>(baseAtk * 0.15 + 0.5);
    EXPECT_EQ(hero.attack(), baseAtk + swordBonus);
    EXPECT_EQ(hero.equippedItem(), ItemType::kSword);
}

TEST(StarUpgradeTest, EquipArmorAddsPhysDef) {
    AshRaiderHero hero(1, UnitOwner::player);
    const int baseDef = hero.basePhysicalDef();
    hero.equipItem(ItemType::kArmor);
    const int armorBonus = static_cast<int>(baseDef * 0.25 + 0.5);
    EXPECT_EQ(hero.physicalDef(), baseDef + armorBonus);
    EXPECT_EQ(hero.maxHp(), hero.baseMaxHp());
}

TEST(StarUpgradeTest, UnequipRemovesBonus) {
    AshRaiderHero hero(1, UnitOwner::player);
    const int baseAtk = hero.attack();
    hero.equipItem(ItemType::kSword);
    hero.unequipItem();
    EXPECT_EQ(hero.attack(), baseAtk);
    EXPECT_EQ(hero.equippedItem(), ItemType::kNone);
}

TEST(StarUpgradeTest, UnequipItemAtSpecificSlot) {
    AshRaiderHero hero(1, UnitOwner::player);
    hero.upgradeToStar(2);
    hero.equipItem(ItemType::kSword);
    hero.equipItem(ItemType::kArmor);
    EXPECT_EQ(hero.equippedItems().size(), static_cast<std::size_t>(2));

    hero.unequipItemAt(1);
    EXPECT_EQ(hero.equippedItems().size(), static_cast<std::size_t>(1));
    EXPECT_EQ(hero.equippedItem(), ItemType::kSword);
}

TEST(StarUpgradeTest, EquipReplacesExistingItem) {
    AshRaiderHero hero(1, UnitOwner::player);
    const int basePhysAtk = hero.physicalAtk();
    hero.equipItem(ItemType::kSword);
    hero.equipItem(ItemType::kRing);
    EXPECT_EQ(hero.physicalAtk(), basePhysAtk);
    EXPECT_EQ(hero.magicAtk(), 0);
    EXPECT_EQ(hero.equippedItem(), ItemType::kRing);
}

TEST(StarUpgradeTest, Star2HasTwoEquipSlots) {
    AshRaiderHero hero(1, UnitOwner::player);
    hero.upgradeToStar(2);
    EXPECT_EQ(hero.equipSlotCount(), 2);
}

TEST(StarUpgradeTest, Star2CanEquipTwoItems) {
    AshRaiderHero hero(1, UnitOwner::player);
    hero.upgradeToStar(2);
    const int baseAtk = hero.basePhysicalAtk();
    const int baseHp = hero.baseMaxHp();
    hero.equipItem(ItemType::kSword);
    hero.equipItem(ItemType::kTalisman);

    EXPECT_EQ(hero.equippedItems().size(), static_cast<std::size_t>(2));
    EXPECT_EQ(hero.physicalAtk(), baseAtk + static_cast<int>(baseAtk * 0.15 + 0.5));
    EXPECT_EQ(hero.maxHp(), baseHp + static_cast<int>(baseHp * 0.20 + 0.5));
}

TEST(StarUpgradeTest, EquipBonusDoesNotIncludeSynergy) {
    AshRaiderHero hero(1, UnitOwner::player);
    hero.equipItem(ItemType::kSword);
    hero.setSynergyBuffs(70, 0, 0, 0, 0, false, false, false);
    const int baseAtk = hero.basePhysicalAtk();
    const int swordBonus = static_cast<int>(baseAtk * 0.15 + 0.5);
    EXPECT_EQ(hero.physicalAtk(), baseAtk + swordBonus + 70);
}

TEST(StarUpgradeTest, SwiftGlovesIncreaseAttackSpeed) {
    AshRaiderHero hero(1, UnitOwner::player);
    EXPECT_EQ(hero.attackSpeed(), 100);
    hero.equipItem(ItemType::kSwiftGloves);
    EXPECT_EQ(hero.attackSpeed(), 115);
}

TEST(StarUpgradeTest, BlueCrystalReducesMaxMana) {
    AshRaiderHero hero(1, UnitOwner::player);
    EXPECT_EQ(hero.maxMana(), 75);
    hero.equipItem(ItemType::kBlueCrystal);
    EXPECT_EQ(hero.maxMana(), 45);
}

TEST(StarUpgradeTest, TakeAllEquippedItemsReturnsAndClears) {
    AshRaiderHero hero(1, UnitOwner::player);
    hero.equipItem(ItemType::kSword);
    const std::vector<ItemType> items = hero.takeAllEquippedItems();
    EXPECT_EQ(items.size(), static_cast<std::size_t>(1));
    EXPECT_EQ(items.at(0), ItemType::kSword);
    EXPECT_TRUE(hero.equippedItems().empty());
    EXPECT_EQ(hero.attack(), hero.basePhysicalAtk());
}

TEST(StarUpgradeTest, MergeReturnsConsumedUnitEquipment) {
    Board board(8, 8, 8);
    Player player(1, 0, 100, 1, 8);
    std::map<int, Unit*> units;
    std::vector<Unit*> playerUnits;
    std::vector<ItemType> returnedItems;

    for (int i = 1; i <= 3; ++i) {
        Unit* hero = new AshRaiderHero(i, UnitOwner::player);
        if (i == 2) {
            hero->equipItem(ItemType::kSword);
        }
        if (i == 3) {
            hero->equipItem(ItemType::kArmor);
        }
        playerUnits.push_back(hero);
        units[i] = hero;
        player.addUnit(i);
        board.placeOnBench(i, i - 1);
    }

    const bool merged = StarUpgrade::tryMergeAll(playerUnits, board, units, player, &returnedItems);
    EXPECT_TRUE(merged);
    EXPECT_EQ(returnedItems.size(), static_cast<std::size_t>(2));
}
