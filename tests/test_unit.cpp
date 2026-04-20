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
    EXPECT_EQ(unit.max_hp(), 800);
    EXPECT_EQ(unit.hp(), 800);
    EXPECT_EQ(unit.attack(), 65);
    EXPECT_EQ(unit.attack_range(), 1);
    EXPECT_EQ(unit.max_mana(), 100);
    EXPECT_EQ(unit.mana(), 0);
}

TEST(UnitTest, TakesDamageUntilDeathWithoutGoingNegative) {
    WarriorUnit unit(1, UnitOwner::player);

    unit.take_damage(200);
    EXPECT_EQ(unit.hp(), 600);
    EXPECT_TRUE(unit.is_alive());

    unit.take_damage(10000);
    EXPECT_EQ(unit.hp(), 0);
    EXPECT_FALSE(unit.is_alive());
}

TEST(UnitTest, GainsManaWithUpperBound) {
    MageUnit unit(2, UnitOwner::enemy);

    unit.gain_mana(30);
    EXPECT_EQ(unit.mana(), 30);

    unit.gain_mana(500);
    EXPECT_EQ(unit.mana(), unit.max_mana());
}

TEST(UnitTest, RejectsInvalidStatsAtConstruction) {
    EXPECT_THROW(Unit(-1, "InvalidId", UnitOwner::player, 100, 10, 1, 100), std::invalid_argument);
    EXPECT_THROW(Unit(1, "InvalidHp", UnitOwner::player, 0, 10, 1, 100), std::invalid_argument);
    EXPECT_THROW(Unit(1, "InvalidRange", UnitOwner::player, 100, 10, 0, 100), std::invalid_argument);
}

TEST(UnitTest, DeadUnitCannotGainManaOrTakeMoreDamage) {
    WarriorUnit unit(9, UnitOwner::player);
    unit.take_damage(9999);
    const int dead_hp = unit.hp();
    const int dead_mana = unit.mana();

    unit.take_damage(10);
    unit.gain_mana(50);

    EXPECT_EQ(unit.hp(), dead_hp);
    EXPECT_EQ(unit.mana(), dead_mana);
}
