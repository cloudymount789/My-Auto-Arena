#include <gtest/gtest.h>

#include <map>
#include <vector>

#include "core/Board.h"
#include "core/HeroUnits.h"
#include "core/SynergySystem.h"
#include "core/Unit.h"

using namespace my_auto_arena::core;

// ── 辅助函数 ──────────────────────────────────────────────────────────────────

static void placeOnPlayerHalf(Board& board, std::map<int, Unit*>& units, Unit* u, int row, int col) {
    board.placeOnBoard(u->id(), Position{row, col});
    units[u->id()] = u;
}

// ── 职业统计测试 ──────────────────────────────────────────────────────────────

TEST(SynergyTest, SingleWarrior_NoSynergy_NoBuffs) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    AshRaiderHero warrior(1, UnitOwner::player);  // 战士 base ATK=62
    placeOnPlayerHalf(board, units, &warrior, 6, 0);

    SynergySystem::applyBuffs(board, units);
    // 单个战士不触发近战羁绊（需要 2 人），ATK 无加成。
    EXPECT_EQ(warrior.attack(), 62);
}

// 近战羁绊2：近战单位 +45 ATK
TEST(SynergyTest, TwoMeleeUnits_AtkBuff45) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    AshRaiderHero warrior(1, UnitOwner::player);   // base ATK=62
    CurseHammerHero tank(2, UnitOwner::player);    // base ATK=48
    placeOnPlayerHalf(board, units, &warrior, 6, 0);
    placeOnPlayerHalf(board, units, &tank, 6, 1);

    SynergySystem::applyBuffs(board, units);

    // 近战 2 人羁绊：近战单位 +45 ATK（不给 HP）。
    EXPECT_EQ(warrior.attack(), 62 + 45);
    EXPECT_EQ(tank.attack(), 48 + 45);
    // HP 不变（2近战只给 ATK）。
    EXPECT_EQ(warrior.maxHp(), 1600);
    EXPECT_EQ(tank.maxHp(), 2600);
}

// 近战羁绊4：近战单位 +110 ATK +500 HP
TEST(SynergyTest, FourMeleeUnits_AtkBuff110_HpBuff500) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    AshRaiderHero w1(1, UnitOwner::player);
    AshRaiderHero w2(2, UnitOwner::player);
    CurseHammerHero t1(3, UnitOwner::player);
    CurseHammerHero t2(4, UnitOwner::player);
    placeOnPlayerHalf(board, units, &w1, 6, 0);
    placeOnPlayerHalf(board, units, &w2, 6, 1);
    placeOnPlayerHalf(board, units, &t1, 6, 2);
    placeOnPlayerHalf(board, units, &t2, 6, 3);

    SynergySystem::applyBuffs(board, units);

    EXPECT_EQ(w1.attack(), 62 + 110);
    EXPECT_EQ(t1.attack(), 48 + 110);
    EXPECT_EQ(w1.maxHp(), 1600 + 500);
    EXPECT_EQ(t1.maxHp(), 2600 + 500);
}

// 弓手羁绊2：射手 +100 ATK
TEST(SynergyTest, TwoArchers_AtkBuff100) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    NightArcherHero a1(1, UnitOwner::player);  // base ATK=60
    NightArcherHero a2(2, UnitOwner::player);
    placeOnPlayerHalf(board, units, &a1, 6, 0);
    placeOnPlayerHalf(board, units, &a2, 6, 1);

    SynergySystem::applyBuffs(board, units);

    EXPECT_EQ(a1.attack(), 60 + 100);
    EXPECT_EQ(a2.attack(), 60 + 100);
}

// 弓手羁绊3：射手 +260 ATK
TEST(SynergyTest, ThreeArchers_AtkBuff260) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    NightArcherHero a1(1, UnitOwner::player);
    NightArcherHero a2(2, UnitOwner::player);
    NightArcherHero a3(3, UnitOwner::player);
    placeOnPlayerHalf(board, units, &a1, 6, 0);
    placeOnPlayerHalf(board, units, &a2, 6, 1);
    placeOnPlayerHalf(board, units, &a3, 6, 2);

    SynergySystem::applyBuffs(board, units);

    EXPECT_EQ(a1.attack(), 60 + 260);
}

// 法术羁绊1：法师 +120 ATK
TEST(SynergyTest, OneMage_AtkBuff120) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    MistWitchHero mage(1, UnitOwner::player);  // base ATK=38
    placeOnPlayerHalf(board, units, &mage, 6, 0);

    SynergySystem::applyBuffs(board, units);

    EXPECT_EQ(mage.attack(), 38 + 120);
}

// 法术羁绊2：法师 +300 ATK
TEST(SynergyTest, TwoMages_AtkBuff300) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    MistWitchHero m1(1, UnitOwner::player);
    MistWitchHero m2(2, UnitOwner::player);
    placeOnPlayerHalf(board, units, &m1, 6, 0);
    placeOnPlayerHalf(board, units, &m2, 6, 1);

    SynergySystem::applyBuffs(board, units);

    EXPECT_EQ(m1.attack(), 38 + 300);
    EXPECT_EQ(m2.attack(), 38 + 300);
}

// 圣愈羁绊1：全体 +800 HP
TEST(SynergyTest, OneHealer_AllHpBuff800) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    BonePrayerHero healer(1, UnitOwner::player);   // base HP=1400
    AshRaiderHero warrior(2, UnitOwner::player);   // base HP=1600
    placeOnPlayerHalf(board, units, &healer, 6, 0);
    placeOnPlayerHalf(board, units, &warrior, 6, 1);

    SynergySystem::applyBuffs(board, units);

    EXPECT_EQ(healer.maxHp(), 1400 + 800);
    EXPECT_EQ(warrior.maxHp(), 1600 + 800);
}

// 圣愈羁绊2：全体 +2000 HP
TEST(SynergyTest, TwoHealers_AllHpBuff2000) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    BonePrayerHero h1(1, UnitOwner::player);
    BonePrayerHero h2(2, UnitOwner::player);
    AshRaiderHero warrior(3, UnitOwner::player);
    placeOnPlayerHalf(board, units, &h1, 6, 0);
    placeOnPlayerHalf(board, units, &h2, 6, 1);
    placeOnPlayerHalf(board, units, &warrior, 6, 2);

    SynergySystem::applyBuffs(board, units);

    EXPECT_EQ(warrior.maxHp(), 1600 + 2000);
    EXPECT_EQ(h1.maxHp(), 1400 + 2000);
}

// ── clearBuffs 测试 ───────────────────────────────────────────────────────────

TEST(SynergyTest, ClearBuffsResetsToBase) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    AshRaiderHero warrior(1, UnitOwner::player);
    CurseHammerHero tank(2, UnitOwner::player);
    placeOnPlayerHalf(board, units, &warrior, 6, 0);
    placeOnPlayerHalf(board, units, &tank, 6, 1);

    SynergySystem::applyBuffs(board, units);
    EXPECT_GT(warrior.attack(), 62);  // 近战羁绊2后有 ATK 加成

    std::vector<Unit*> playerUnits = {&warrior, &tank};
    SynergySystem::clearBuffs(playerUnits);

    EXPECT_EQ(warrior.attack(), 62);
    EXPECT_EQ(warrior.maxHp(), 1600);
    EXPECT_EQ(tank.attack(), 48);
    EXPECT_EQ(tank.maxHp(), 2600);
}

// ── getActiveSynergies 测试 ───────────────────────────────────────────────────

TEST(SynergyTest, GetActiveSynergies_MeleeActive) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    AshRaiderHero w1(1, UnitOwner::player);
    CurseHammerHero t1(2, UnitOwner::player);
    placeOnPlayerHalf(board, units, &w1, 6, 0);
    placeOnPlayerHalf(board, units, &t1, 6, 1);

    const std::vector<ActiveSynergy> synergies =
        SynergySystem::getActiveSynergies(board, units);

    bool meleeFound = false;
    for (std::size_t i = 0; i < synergies.size(); ++i) {
        if (synergies.at(i).name == "近战" && synergies.at(i).activeThreshold > 0) {
            meleeFound = true;
        }
    }
    EXPECT_TRUE(meleeFound);
}

// ── 敌方单位不受羁绊影响 ──────────────────────────────────────────────────────

TEST(SynergyTest, EnemyUnitsNotBuffed) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    AshRaiderHero enemyWarrior(1, UnitOwner::enemy);
    board.placeOnBoard(1, Position{0, 0});  // 敌方半场
    units[1] = &enemyWarrior;

    SynergySystem::applyBuffs(board, units);
    EXPECT_EQ(enemyWarrior.attack(), 62);   // 无加成
    EXPECT_EQ(enemyWarrior.maxHp(), 1600);  // 无加成
}
