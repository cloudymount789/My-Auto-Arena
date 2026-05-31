#include "core/HeroUnits.h"

#include <gtest/gtest.h>

#include <map>
#include <string>

#include "core/Board.h"
#include "core/Unit.h"

using my_auto_arena::core::AshRaiderHero;
using my_auto_arena::core::Board;
using my_auto_arena::core::CurseHammerHero;
using my_auto_arena::core::MistWitchHero;
using my_auto_arena::core::NightArcherHero;
using my_auto_arena::core::Position;
using my_auto_arena::core::UnitOwner;
using my_auto_arena::core::WarriorUnit;

TEST(HeroSkillTest, AshRaiderSkillDamagesAndStunsPrimaryTarget) {
    Board board(4, 4, 1);
    AshRaiderHero hero(1, UnitOwner::player);
    WarriorUnit victim(2, UnitOwner::enemy);
    std::map<int, my_auto_arena::core::Unit*> reg;
    reg[hero.id()] = &hero;
    reg[victim.id()] = &victim;
    hero.gainMana(hero.maxMana());
    const int before = victim.hp();
    const int expectedRaw = hero.physicalAtk() * 150 / 100;
    hero.castFullManaSkill(board, reg, &victim);
    EXPECT_EQ(victim.hp(), before - std::max(1, expectedRaw - victim.physicalDef()));
    EXPECT_EQ(hero.mana(), 0);
    EXPECT_EQ(victim.stunTicksRemaining(), 2);
}

TEST(HeroSkillTest, ArcherLineSkillHitsEnemiesOnSameRow) {
    Board board(8, 8, 1);
    NightArcherHero hero(1, UnitOwner::player);
    WarriorUnit victimA(2, UnitOwner::enemy);
    WarriorUnit victimB(3, UnitOwner::enemy);
    WarriorUnit offLine(4, UnitOwner::enemy);
    std::map<int, my_auto_arena::core::Unit*> reg;
    reg[hero.id()] = &hero;
    reg[victimA.id()] = &victimA;
    reg[victimB.id()] = &victimB;
    reg[offLine.id()] = &offLine;
    ASSERT_TRUE(board.placeOnBoard(hero.id(), Position{4, 2}));
    ASSERT_TRUE(board.placeOnBoard(victimA.id(), Position{4, 5}));
    ASSERT_TRUE(board.placeOnBoard(victimB.id(), Position{4, 7}));
    ASSERT_TRUE(board.placeOnBoard(offLine.id(), Position{2, 5}));
    hero.gainMana(hero.maxMana());
    const int dmg = std::max(1, hero.physicalAtk() * 200 / 100 - victimA.physicalDef());
    const int beforeA = victimA.hp();
    const int beforeB = victimB.hp();
    const int beforeOff = offLine.hp();
    hero.castFullManaSkill(board, reg, &victimA);
    EXPECT_EQ(victimA.hp(), beforeA - dmg);
    EXPECT_EQ(victimB.hp(), beforeB - dmg);
    EXPECT_EQ(offLine.hp(), beforeOff);
}

TEST(HeroSkillTest, MageRangeSkillHitsAllEnemiesInRange) {
    Board board(8, 8, 1);
    MistWitchHero hero(1, UnitOwner::player);
    WarriorUnit inRange(2, UnitOwner::enemy);
    WarriorUnit outRange(3, UnitOwner::enemy);
    std::map<int, my_auto_arena::core::Unit*> reg;
    reg[hero.id()] = &hero;
    reg[inRange.id()] = &inRange;
    reg[outRange.id()] = &outRange;
    ASSERT_TRUE(board.placeOnBoard(hero.id(), Position{4, 4}));
    ASSERT_TRUE(board.placeOnBoard(inRange.id(), Position{4, 6}));
    ASSERT_TRUE(board.placeOnBoard(outRange.id(), Position{0, 0}));
    hero.gainMana(hero.maxMana());
    const int dmg = std::max(1, hero.magicAtk() * 200 / 100 - inRange.magicDef());
    const int beforeIn = inRange.hp();
    const int beforeOut = outRange.hp();
    hero.castFullManaSkill(board, reg, &inRange);
    EXPECT_EQ(inRange.hp(), beforeIn - dmg);
    EXPECT_EQ(outRange.hp(), beforeOut);
}

TEST(HeroSkillTest, HammerSkillHitsAdjacentEnemy) {
    Board board(4, 4, 1);
    CurseHammerHero hero(1, UnitOwner::player);
    WarriorUnit victim(2, UnitOwner::enemy);
    std::map<int, my_auto_arena::core::Unit*> reg;
    reg[hero.id()] = &hero;
    reg[victim.id()] = &victim;
    ASSERT_TRUE(board.placeOnBoard(hero.id(), Position{2, 2}));
    ASSERT_TRUE(board.placeOnBoard(victim.id(), Position{2, 3}));
    hero.gainMana(hero.maxMana());
    const int before = victim.hp();
    hero.castFullManaSkill(board, reg, nullptr);
    EXPECT_EQ(victim.hp(), before - 220);
}

TEST(HeroSkillTest, SkillDescriptionsAreAvailable) {
    EXPECT_NE(std::string(my_auto_arena::core::skillDescriptionForHeroType(
                  my_auto_arena::core::HeroType::kHealer))
                  .find("15%"),
              std::string::npos);
    EXPECT_NE(std::string(my_auto_arena::core::skillDescriptionForUnitClass(
                  my_auto_arena::core::UnitClass::kWarrior))
                  .find("150%"),
              std::string::npos);
}
