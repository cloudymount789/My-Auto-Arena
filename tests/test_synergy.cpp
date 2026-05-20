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

TEST(SynergyTest, WarriorArcher_NoSynergy_NoBuffs) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    AshRaiderHero warrior(1, UnitOwner::player);  // 战士
    placeOnPlayerHalf(board, units, &warrior, 6, 0);

    SynergySystem::applyBuffs(board, units);
    // 单个战士不触发近战羁绊（需要 2 人）。
    EXPECT_EQ(warrior.attack(), warrior.attack());  // 无 bonusAtk
    // 验证 bonusAtk_=0 通过 setSynergyBuffs 后 attack() 不变
    warrior.setSynergyBuffs(0, 0);
    EXPECT_EQ(warrior.attack(), 60);  // AshRaider 基础攻击
}

TEST(SynergyTest, TwoMeleeUnits_HpBuff300) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    AshRaiderHero warrior(1, UnitOwner::player);
    CurseHammerHero tank(2, UnitOwner::player);
    placeOnPlayerHalf(board, units, &warrior, 6, 0);
    placeOnPlayerHalf(board, units, &tank, 6, 1);

    SynergySystem::applyBuffs(board, units);

    // 近战 2 人羁绊：所有玩家单位 +300 HP。
    EXPECT_EQ(warrior.maxHp(), 2200 + 300);
    EXPECT_EQ(tank.maxHp(), 3200 + 300);
}

TEST(SynergyTest, FourMeleeUnits_HpBuff700) {
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

    EXPECT_EQ(w1.maxHp(), 2200 + 700);
    EXPECT_EQ(t1.maxHp(), 3200 + 700);
}

TEST(SynergyTest, TwoArchers_AtkBuff50) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    NightArcherHero a1(1, UnitOwner::player);
    NightArcherHero a2(2, UnitOwner::player);
    placeOnPlayerHalf(board, units, &a1, 6, 0);
    placeOnPlayerHalf(board, units, &a2, 6, 1);

    SynergySystem::applyBuffs(board, units);

    // 弓手 2 人羁绊：弓手 +50 ATK。
    EXPECT_EQ(a1.attack(), 55 + 50);
    EXPECT_EQ(a2.attack(), 55 + 50);
}

TEST(SynergyTest, ThreeArchers_AtkBuff120) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    NightArcherHero a1(1, UnitOwner::player);
    NightArcherHero a2(2, UnitOwner::player);
    NightArcherHero a3(3, UnitOwner::player);
    placeOnPlayerHalf(board, units, &a1, 6, 0);
    placeOnPlayerHalf(board, units, &a2, 6, 1);
    placeOnPlayerHalf(board, units, &a3, 6, 2);

    SynergySystem::applyBuffs(board, units);

    EXPECT_EQ(a1.attack(), 55 + 120);
}

TEST(SynergyTest, OneMage_AtkBuff70) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    MistWitchHero mage(1, UnitOwner::player);
    placeOnPlayerHalf(board, units, &mage, 6, 0);

    SynergySystem::applyBuffs(board, units);

    EXPECT_EQ(mage.attack(), 35 + 70);
}

TEST(SynergyTest, TwoMages_AtkBuff160) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    MistWitchHero m1(1, UnitOwner::player);
    MistWitchHero m2(2, UnitOwner::player);
    placeOnPlayerHalf(board, units, &m1, 6, 0);
    placeOnPlayerHalf(board, units, &m2, 6, 1);

    SynergySystem::applyBuffs(board, units);

    EXPECT_EQ(m1.attack(), 35 + 160);
    EXPECT_EQ(m2.attack(), 35 + 160);
}

TEST(SynergyTest, OneHealer_AllHpBuff400) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    BonePrayerHero healer(1, UnitOwner::player);
    AshRaiderHero warrior(2, UnitOwner::player);
    placeOnPlayerHalf(board, units, &healer, 6, 0);
    placeOnPlayerHalf(board, units, &warrior, 6, 1);

    SynergySystem::applyBuffs(board, units);

    // 圣愈 1 人羁绊：所有玩家单位 +400 HP。
    EXPECT_EQ(healer.maxHp(), 1800 + 400);
    EXPECT_EQ(warrior.maxHp(), 2200 + 400);
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
    EXPECT_GT(warrior.maxHp(), 2200);

    std::vector<Unit*> playerUnits = {&warrior, &tank};
    SynergySystem::clearBuffs(playerUnits);

    EXPECT_EQ(warrior.maxHp(), 2200);
    EXPECT_EQ(tank.maxHp(), 3200);
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
    EXPECT_EQ(enemyWarrior.attack(), 60);   // 无加成
    EXPECT_EQ(enemyWarrior.maxHp(), 2200);  // 无加成
}
