#include "core/Unit.h"

#include <gtest/gtest.h>
#include <stdexcept>

using my_auto_arena::core::Unit;
using my_auto_arena::core::MageUnit;
using my_auto_arena::core::UnitOwner;
using my_auto_arena::core::WarriorUnit;

TEST(UnitTest, WarriorHasExpectedDefaultStats) {
    const WarriorUnit unit(1, UnitOwner::player);

    EXPECT_EQ(unit.id(), 1);
    EXPECT_EQ(unit.name(), "Warrior");
    EXPECT_EQ(unit.maxHp(), 800);
    EXPECT_EQ(unit.hp(), 800);
    EXPECT_EQ(unit.attack(), 65);
    EXPECT_EQ(unit.attackRange(), 1);
    EXPECT_EQ(unit.maxMana(), 100);
    EXPECT_EQ(unit.mana(), 0);
}

TEST(UnitTest, TakesDamageUntilDeathWithoutGoingNegative) {
    WarriorUnit unit(1, UnitOwner::player);

    unit.takeDamage(200);
    EXPECT_EQ(unit.hp(), 600);
    EXPECT_TRUE(unit.isAlive());

    unit.takeDamage(10000);
    EXPECT_EQ(unit.hp(), 0);
    EXPECT_FALSE(unit.isAlive());
}

TEST(UnitTest, GainsManaWithUpperBound) {
    MageUnit unit(2, UnitOwner::enemy);

    unit.gainMana(30);
    EXPECT_EQ(unit.mana(), 30);

    unit.gainMana(500);
    EXPECT_EQ(unit.mana(), unit.maxMana());
}

TEST(UnitTest, RejectsInvalidStatsAtConstruction) {
    EXPECT_THROW(Unit(-1, "InvalidId", UnitOwner::player, 100, 10, 1, 100), std::invalid_argument);
    EXPECT_THROW(Unit(1, "InvalidHp", UnitOwner::player, 0, 10, 1, 100), std::invalid_argument);
    EXPECT_THROW(Unit(1, "InvalidRange", UnitOwner::player, 100, 10, 0, 100), std::invalid_argument);
}

TEST(UnitTest, DeadUnitCannotGainManaOrTakeMoreDamage) {
    WarriorUnit unit(9, UnitOwner::player);
    unit.takeDamage(9999);
    const int dead_hp = unit.hp();
    const int dead_mana = unit.mana();

    unit.takeDamage(10);
    unit.gainMana(50);

    EXPECT_EQ(unit.hp(), dead_hp);
    EXPECT_EQ(unit.mana(), dead_mana);
}
