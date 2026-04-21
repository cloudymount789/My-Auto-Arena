#include "core/HeroUnits.h"

#include <gtest/gtest.h>

#include <map>

#include "core/Board.h"
#include "core/Unit.h"

using my_auto_arena::core::AshRaiderHero;
using my_auto_arena::core::Board;
using my_auto_arena::core::CurseHammerHero;
using my_auto_arena::core::Position;
using my_auto_arena::core::UnitOwner;
using my_auto_arena::core::WarriorUnit;

TEST(HeroSkillTest, AshRaiderSkillDamagesPrimaryTarget) {
    Board board(4, 4, 1);
    AshRaiderHero hero(1, UnitOwner::player);
    WarriorUnit victim(2, UnitOwner::enemy);
    std::map<int, my_auto_arena::core::Unit*> reg;
    reg[hero.id()] = &hero;
    reg[victim.id()] = &victim;
    hero.gainMana(hero.maxMana());
    const int before = victim.hp();
    hero.castFullManaSkill(board, reg, &victim);
    EXPECT_EQ(victim.hp(), before - 180);
    EXPECT_EQ(hero.mana(), 0);
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
    EXPECT_EQ(victim.hp(), before - 120);
}
