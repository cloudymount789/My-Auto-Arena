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
    // 单个★1战士贡献 1 点，T1 需 3 点，无加成。
    EXPECT_EQ(warrior.attack(), 62);
}

// 近战 T1：星级点数 >= 3，近战单位 +70 ATK
// 3 个★1 = 3 点，恰好触发 T1
TEST(SynergyTest, TwoMeleeUnits_AtkBuff45) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    AshRaiderHero warrior(1, UnitOwner::player);   // base ATK=62
    CurseHammerHero tank(2, UnitOwner::player);    // base ATK=48
    AshRaiderHero warrior2(3, UnitOwner::player);  // 第3个近战，凑满 3 点
    placeOnPlayerHalf(board, units, &warrior,  6, 0);
    placeOnPlayerHalf(board, units, &tank,     6, 1);
    placeOnPlayerHalf(board, units, &warrior2, 6, 2);

    SynergySystem::applyBuffs(board, units);

    // 近战 3 点羁绊（T1）：近战单位 +70 ATK，无 HP 加成。
    EXPECT_EQ(warrior.attack(),  62 + 70);
    EXPECT_EQ(tank.attack(),     48 + 70);
    EXPECT_EQ(warrior.maxHp(),  1600);
    EXPECT_EQ(tank.maxHp(),     2600);
}

// 近战 T2：星级点数 >= 6，近战单位 +180 ATK +1000 HP
// 使用 3 个★2 = 6 点触发 T2
TEST(SynergyTest, FourMeleeUnits_AtkBuff110_HpBuff500) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    // 3 个★2 战士/坦克：每个★2=3点，共 9 点，触发 T2
    AshRaiderHero w1(1, UnitOwner::player);
    AshRaiderHero w2(2, UnitOwner::player);
    CurseHammerHero t1(3, UnitOwner::player);
    placeOnPlayerHalf(board, units, &w1, 6, 0);
    placeOnPlayerHalf(board, units, &w2, 6, 1);
    placeOnPlayerHalf(board, units, &t1, 6, 2);
    w1.upgradeToStar(2);
    w2.upgradeToStar(2);
    t1.upgradeToStar(2);

    SynergySystem::applyBuffs(board, units);

    // T2 收益：+180 ATK +1000 HP
    EXPECT_EQ(w1.attack(), static_cast<int>(62 * 3.0) + 180);
    EXPECT_EQ(t1.attack(), static_cast<int>(48 * 3.0) + 180);
    EXPECT_EQ(w1.maxHp(),  static_cast<int>(1600 * 3.0) + 1000);
    EXPECT_EQ(t1.maxHp(),  static_cast<int>(2600 * 3.0) + 1000);
}

// 弓手 T1：星级点数 >= 3，射手 +160 ATK
TEST(SynergyTest, TwoArchers_AtkBuff100) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    NightArcherHero a1(1, UnitOwner::player);  // base ATK=60
    NightArcherHero a2(2, UnitOwner::player);
    NightArcherHero a3(3, UnitOwner::player);
    placeOnPlayerHalf(board, units, &a1, 6, 0);
    placeOnPlayerHalf(board, units, &a2, 6, 1);
    placeOnPlayerHalf(board, units, &a3, 6, 2);

    SynergySystem::applyBuffs(board, units);

    // 3 个★1射手 = 3 点，触发 T1：+160 ATK
    EXPECT_EQ(a1.attack(), 60 + 160);
    EXPECT_EQ(a2.attack(), 60 + 160);
}

// 弓手 T2：星级点数 >= 6，射手 +400 ATK
TEST(SynergyTest, ThreeArchers_AtkBuff260) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    NightArcherHero a1(1, UnitOwner::player);
    NightArcherHero a2(2, UnitOwner::player);
    NightArcherHero a3(3, UnitOwner::player);
    placeOnPlayerHalf(board, units, &a1, 6, 0);
    placeOnPlayerHalf(board, units, &a2, 6, 1);
    placeOnPlayerHalf(board, units, &a3, 6, 2);
    a1.upgradeToStar(2);
    a2.upgradeToStar(2);
    a3.upgradeToStar(2);

    SynergySystem::applyBuffs(board, units);

    // 3 个★2射手 = 6 点，触发 T2：+400 ATK
    EXPECT_EQ(a1.attack(), static_cast<int>(60 * 3.0) + 400);
}

// 法术 T1：3 个★1法师 = 3 点，+180 法术攻击（走 magicAtk 通道，不影响 attack()）
TEST(SynergyTest, OneMage_AtkBuff120) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    MistWitchHero m1(1, UnitOwner::player);  // baseMagicAtk=38
    MistWitchHero m2(2, UnitOwner::player);
    MistWitchHero m3(3, UnitOwner::player);
    placeOnPlayerHalf(board, units, &m1, 6, 0);
    placeOnPlayerHalf(board, units, &m2, 6, 1);
    placeOnPlayerHalf(board, units, &m3, 6, 2);

    SynergySystem::applyBuffs(board, units);

    // 法术羁绊加成仅提升 magicAtk()，物理 attack() 保持基础值。
    EXPECT_EQ(m1.magicAtk(), 38 + 180);
    EXPECT_EQ(m1.attack(),   38);       // 物理攻击不受法术羁绊影响
}

// 法术 T2：3 个★2法师 = 6 点，+450 法术攻击（升星后 baseMagicAtk 也随之缩放）
TEST(SynergyTest, TwoMages_AtkBuff300) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    MistWitchHero m1(1, UnitOwner::player);
    MistWitchHero m2(2, UnitOwner::player);
    MistWitchHero m3(3, UnitOwner::player);
    placeOnPlayerHalf(board, units, &m1, 6, 0);
    placeOnPlayerHalf(board, units, &m2, 6, 1);
    placeOnPlayerHalf(board, units, &m3, 6, 2);
    m1.upgradeToStar(2);
    m2.upgradeToStar(2);
    m3.upgradeToStar(2);

    SynergySystem::applyBuffs(board, units);

    // ★2 后 baseMagicAtk = 38*3=114；法术羁绊 T2 再加 +450
    EXPECT_EQ(m1.magicAtk(), static_cast<int>(38 * 3.0) + 450);
    EXPECT_EQ(m2.magicAtk(), static_cast<int>(38 * 3.0) + 450);
    // 物理攻击也随升星缩放，但不受法术羁绊影响
    EXPECT_EQ(m1.attack(), static_cast<int>(38 * 3.0));
}

// 圣愈 T1：3 个★1治疗师 = 3 点，全体 +1000 HP
TEST(SynergyTest, OneHealer_AllHpBuff800) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    BonePrayerHero h1(1, UnitOwner::player);   // base HP=1400
    BonePrayerHero h2(2, UnitOwner::player);
    BonePrayerHero h3(3, UnitOwner::player);
    AshRaiderHero warrior(4, UnitOwner::player);   // base HP=1600
    placeOnPlayerHalf(board, units, &h1,      6, 0);
    placeOnPlayerHalf(board, units, &h2,      6, 1);
    placeOnPlayerHalf(board, units, &h3,      6, 2);
    placeOnPlayerHalf(board, units, &warrior, 6, 3);

    SynergySystem::applyBuffs(board, units);

    // T1：全体 +1000 HP
    EXPECT_EQ(h1.maxHp(),      1400 + 1000);
    EXPECT_EQ(warrior.maxHp(), 1600 + 1000);
}

// 圣愈 T2：3 个★2治疗师 = 6 点，全体 +2500 HP
TEST(SynergyTest, TwoHealers_AllHpBuff2000) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    BonePrayerHero h1(1, UnitOwner::player);
    BonePrayerHero h2(2, UnitOwner::player);
    BonePrayerHero h3(3, UnitOwner::player);
    AshRaiderHero warrior(4, UnitOwner::player);
    placeOnPlayerHalf(board, units, &h1,      6, 0);
    placeOnPlayerHalf(board, units, &h2,      6, 1);
    placeOnPlayerHalf(board, units, &h3,      6, 2);
    placeOnPlayerHalf(board, units, &warrior, 6, 3);
    h1.upgradeToStar(2);
    h2.upgradeToStar(2);
    h3.upgradeToStar(2);

    SynergySystem::applyBuffs(board, units);

    // T2：全体 +2500 HP
    EXPECT_EQ(warrior.maxHp(), 1600 + 2500);
    EXPECT_EQ(h1.maxHp(),      static_cast<int>(1400 * 3.0) + 2500);
}

// ── clearBuffs 测试 ───────────────────────────────────────────────────────────

TEST(SynergyTest, ClearBuffsResetsToBase) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    // 3 个★1近战触发 T1
    AshRaiderHero warrior(1, UnitOwner::player);
    CurseHammerHero tank(2, UnitOwner::player);
    AshRaiderHero warrior2(3, UnitOwner::player);
    placeOnPlayerHalf(board, units, &warrior,  6, 0);
    placeOnPlayerHalf(board, units, &tank,     6, 1);
    placeOnPlayerHalf(board, units, &warrior2, 6, 2);

    SynergySystem::applyBuffs(board, units);
    EXPECT_GT(warrior.attack(), 62);  // T1 近战羁绊后有 ATK 加成

    std::vector<Unit*> playerUnits = {&warrior, &tank, &warrior2};
    SynergySystem::clearBuffs(playerUnits);

    EXPECT_EQ(warrior.attack(),  62);
    EXPECT_EQ(warrior.maxHp(),  1600);
    EXPECT_EQ(tank.attack(),     48);
    EXPECT_EQ(tank.maxHp(),     2600);
}

// ── getActiveSynergies 测试 ───────────────────────────────────────────────────

TEST(SynergyTest, GetActiveSynergies_MeleeActive) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    // 3 个★1近战 = 3 点，触发 T1
    AshRaiderHero w1(1, UnitOwner::player);
    CurseHammerHero t1(2, UnitOwner::player);
    AshRaiderHero w2(3, UnitOwner::player);
    placeOnPlayerHalf(board, units, &w1, 6, 0);
    placeOnPlayerHalf(board, units, &t1, 6, 1);
    placeOnPlayerHalf(board, units, &w2, 6, 2);

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
