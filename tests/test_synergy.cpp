#include <gtest/gtest.h>

#include <map>
#include <string>
#include <vector>

#include "core/BattleEngine.h"
#include "core/Board.h"
#include "core/HeroUnits.h"
#include "core/Item.h"
#include "core/SynergySystem.h"
#include "core/Unit.h"

using namespace my_auto_arena::core;

static void placeOnPlayerHalf(Board& board, std::map<int, Unit*>& units, Unit* u, int row, int col) {
    board.placeOnBoard(u->id(), Position{row, col});
    units[u->id()] = u;
}

static const ActiveSynergy* findSynergy(const std::vector<ActiveSynergy>& synergies,
                                        const std::string& name) {
    for (std::size_t i = 0; i < synergies.size(); ++i) {
        if (synergies.at(i).name == name) {
            return &synergies.at(i);
        }
    }
    return nullptr;
}

TEST(SynergyTest, SingleWarriorContributesOneStarAndNoBuff) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    AshRaiderHero warrior(1, UnitOwner::player);
    placeOnPlayerHalf(board, units, &warrior, 6, 0);

    SynergySystem::applyBuffs(board, units);

    EXPECT_EQ(warrior.attack(), 62);
    const std::vector<ActiveSynergy> synergies = SynergySystem::getActiveSynergies(board, units);
    const ActiveSynergy* offense = findSynergy(synergies, "进攻就是最好的防守！");
    ASSERT_NE(offense, nullptr);
    EXPECT_EQ(offense->count, 1);
    EXPECT_EQ(offense->activeThreshold, 0);
    EXPECT_EQ(offense->nextThreshold, 3);
}

TEST(SynergyTest, OffenseSynergyBuffsWarriorArcherMageByBaseAttackOnly) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    AshRaiderHero warrior(1, UnitOwner::player);
    NightArcherHero archer(2, UnitOwner::player);
    MistWitchHero mage(3, UnitOwner::player);
    BonePrayerHero healer(4, UnitOwner::player);
    warrior.equipItem(ItemType::kSword);
    placeOnPlayerHalf(board, units, &warrior, 6, 0);
    placeOnPlayerHalf(board, units, &archer, 6, 1);
    placeOnPlayerHalf(board, units, &mage, 6, 2);
    placeOnPlayerHalf(board, units, &healer, 6, 3);

    SynergySystem::applyBuffs(board, units);

    EXPECT_EQ(warrior.attack(), 62 + 9 + 6);
    EXPECT_EQ(archer.attack(), 60 + 6);
    EXPECT_EQ(mage.magicAtk(), 38 + 4);
    EXPECT_EQ(healer.magicAtk(), 28);
}

TEST(SynergyTest, DefenseSynergyBuffsOnlyTanksAndHealers) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    CurseHammerHero tank1(1, UnitOwner::player);
    CurseHammerHero tank2(2, UnitOwner::player);
    BonePrayerHero healer(3, UnitOwner::player);
    AshRaiderHero warrior(4, UnitOwner::player);
    placeOnPlayerHalf(board, units, &tank1, 6, 0);
    placeOnPlayerHalf(board, units, &tank2, 6, 1);
    placeOnPlayerHalf(board, units, &healer, 6, 2);
    placeOnPlayerHalf(board, units, &warrior, 6, 3);

    SynergySystem::applyBuffs(board, units);

    EXPECT_EQ(tank1.physicalDef(), 58);
    EXPECT_EQ(tank1.magicDef(), 17);
    EXPECT_EQ(tank1.maxHp(), 2600);
    EXPECT_EQ(healer.physicalDef(), 6);
    EXPECT_EQ(healer.magicDef(), 22);
    EXPECT_EQ(warrior.physicalDef(), 21);
    EXPECT_EQ(warrior.maxHp(), 1600);
}

TEST(SynergyTest, DefenseSynergyTierTwoAddsDefenseAndHpFromBase) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    CurseHammerHero tank1(1, UnitOwner::player);
    CurseHammerHero tank2(2, UnitOwner::player);
    BonePrayerHero healer(3, UnitOwner::player);
    tank1.upgradeToStar(2);
    tank2.upgradeToStar(2);
    healer.upgradeToStar(2);
    tank1.equipItem(ItemType::kArmor);
    placeOnPlayerHalf(board, units, &tank1, 6, 0);
    placeOnPlayerHalf(board, units, &tank2, 6, 1);
    placeOnPlayerHalf(board, units, &healer, 6, 2);

    SynergySystem::applyBuffs(board, units);

    EXPECT_EQ(tank1.physicalDef(), 150 + 38 + 30 + 8);
    EXPECT_EQ(tank1.magicDef(), 45 + 9);
    EXPECT_EQ(tank1.maxHp(), 7800 + 780);
}

TEST(SynergyTest, PhysicalMindSynergyGrantsArmorBreakAtFifteenStars) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    AshRaiderHero warrior1(1, UnitOwner::player);
    AshRaiderHero warrior2(2, UnitOwner::player);
    NightArcherHero archer1(3, UnitOwner::player);
    NightArcherHero archer2(4, UnitOwner::player);
    CurseHammerHero tank1(5, UnitOwner::player);
    warrior1.upgradeToStar(3);
    warrior2.upgradeToStar(3);
    archer1.upgradeToStar(3);
    archer2.upgradeToStar(3);
    tank1.upgradeToStar(3);
    placeOnPlayerHalf(board, units, &warrior1, 6, 0);
    placeOnPlayerHalf(board, units, &warrior2, 6, 1);
    placeOnPlayerHalf(board, units, &archer1, 6, 2);
    placeOnPlayerHalf(board, units, &archer2, 6, 3);
    placeOnPlayerHalf(board, units, &tank1, 6, 4);

    SynergySystem::applyBuffs(board, units);

    EXPECT_TRUE(warrior1.hasArmorBreak());
    CurseHammerHero target(100, UnitOwner::enemy);
    target.takePhysicalDamage(200, warrior1.physicalDefenseIgnorePercent());
    EXPECT_EQ(target.hp(), 2600 - (200 - 35));
}

TEST(SynergyTest, MagicSynergyGrantsFullMagicPenetrationAtTwelveStars) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    MistWitchHero mage1(1, UnitOwner::player);
    MistWitchHero mage2(2, UnitOwner::player);
    BonePrayerHero healer1(3, UnitOwner::player);
    BonePrayerHero healer2(4, UnitOwner::player);
    mage1.upgradeToStar(3);
    mage2.upgradeToStar(3);
    healer1.upgradeToStar(3);
    healer2.upgradeToStar(3);
    placeOnPlayerHalf(board, units, &mage1, 6, 0);
    placeOnPlayerHalf(board, units, &mage2, 6, 1);
    placeOnPlayerHalf(board, units, &healer1, 6, 2);
    placeOnPlayerHalf(board, units, &healer2, 6, 3);

    SynergySystem::applyBuffs(board, units);

    EXPECT_TRUE(mage1.hasMagicPenetration());
    MistWitchHero target(100, UnitOwner::enemy);
    target.takeMagicDamage(200, mage1.magicDefenseIgnorePercent());
    EXPECT_EQ(target.hp(), 800);
}

TEST(SynergyTest, ShieldFieldIgnoresOneFullTickAfterFourDamageTicks) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    CurseHammerHero tank1(1, UnitOwner::player);
    CurseHammerHero tank2(2, UnitOwner::player);
    BonePrayerHero healer1(3, UnitOwner::player);
    BonePrayerHero healer2(4, UnitOwner::player);
    AshRaiderHero enemy(100, UnitOwner::enemy);
    tank1.upgradeToStar(3);
    tank2.upgradeToStar(3);
    healer1.upgradeToStar(3);
    healer2.upgradeToStar(3);
    placeOnPlayerHalf(board, units, &tank1, 6, 0);
    placeOnPlayerHalf(board, units, &tank2, 6, 1);
    placeOnPlayerHalf(board, units, &healer1, 6, 2);
    placeOnPlayerHalf(board, units, &healer2, 6, 3);
    board.placeOnBoard(enemy.id(), Position{5, 0});
    units[enemy.id()] = &enemy;

    SynergySystem::applyBuffs(board, units);
    ASSERT_TRUE(tank1.hasShieldField());

    Unit::beginSynergyDamageTick(1);
    tank1.takePhysicalDamage(200);
    Unit::endSynergyDamageTick();
    Unit::beginSynergyDamageTick(2);
    tank1.takePhysicalDamage(200);
    Unit::endSynergyDamageTick();
    Unit::beginSynergyDamageTick(3);
    tank1.takePhysicalDamage(200);
    Unit::endSynergyDamageTick();
    Unit::beginSynergyDamageTick(4);
    tank1.takePhysicalDamage(200);
    Unit::endSynergyDamageTick();

    const int healerHpBefore = healer1.hp();
    Unit::beginSynergyDamageTick(5);
    healer1.takePhysicalDamage(200);
    Unit::endSynergyDamageTick();
    EXPECT_EQ(healer1.hp(), healerHpBefore);

    Unit::beginSynergyDamageTick(6);
    healer1.takePhysicalDamage(200);
    Unit::endSynergyDamageTick();
    EXPECT_LT(healer1.hp(), healerHpBefore);
}

TEST(SynergyTest, ClearBuffsResetsStatsAndSpecialFlags) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    AshRaiderHero warrior(1, UnitOwner::player);
    NightArcherHero archer(2, UnitOwner::player);
    MistWitchHero mage(3, UnitOwner::player);
    placeOnPlayerHalf(board, units, &warrior, 6, 0);
    placeOnPlayerHalf(board, units, &archer, 6, 1);
    placeOnPlayerHalf(board, units, &mage, 6, 2);

    SynergySystem::applyBuffs(board, units);
    EXPECT_GT(warrior.attack(), 62);

    std::vector<Unit*> playerUnits = {&warrior, &archer, &mage};
    SynergySystem::clearBuffs(playerUnits);

    EXPECT_EQ(warrior.attack(), 62);
    EXPECT_FALSE(warrior.hasArmorBreak());
    EXPECT_FALSE(warrior.hasMagicPenetration());
    EXPECT_FALSE(warrior.hasShieldField());
}

TEST(SynergyTest, GetActiveSynergiesReturnsAllSynergiesWithNamesAndDetails) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    const std::vector<ActiveSynergy> synergies = SynergySystem::getActiveSynergies(board, units);

    ASSERT_EQ(synergies.size(), 4U);
    EXPECT_NE(findSynergy(synergies, "进攻就是最好的防守！"), nullptr);
    EXPECT_NE(findSynergy(synergies, "因为太怕痛就全点防御力了"), nullptr);
    EXPECT_NE(findSynergy(synergies, "轻轻敲醒沉睡的心灵"), nullptr);
    EXPECT_NE(findSynergy(synergies, "要用魔法打败魔法"), nullptr);
    for (std::size_t i = 0; i < synergies.size(); ++i) {
        EXPECT_EQ(synergies.at(i).count, 0);
        EXPECT_EQ(synergies.at(i).nextThreshold, 3);
        EXPECT_FALSE(synergies.at(i).classesDescription.empty());
        EXPECT_FALSE(synergies.at(i).detailDescription.empty());
    }
}

TEST(SynergyTest, EnemyUnitsNotBuffed) {
    Board board(8, 8, 8);
    std::map<int, Unit*> units;

    AshRaiderHero enemyWarrior(1, UnitOwner::enemy);
    board.placeOnBoard(1, Position{0, 0});
    units[1] = &enemyWarrior;

    SynergySystem::applyBuffs(board, units);
    EXPECT_EQ(enemyWarrior.attack(), 62);
    EXPECT_EQ(enemyWarrior.maxHp(), 1600);
}
