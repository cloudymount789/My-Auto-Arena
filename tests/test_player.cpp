#include "core/Player.h"

#include <gtest/gtest.h>
#include <stdexcept>

#include "core/Board.h"

using my_auto_arena::core::Board;
using my_auto_arena::core::Player;
using my_auto_arena::core::Position;

TEST(PlayerTest, CreatesWithExpectedDefaults) {
    Player player(1, 10, 100, 1, 3);
    EXPECT_EQ(player.id(), 1);
    EXPECT_EQ(player.gold(), 10);
    EXPECT_EQ(player.hp(), 100);
    EXPECT_EQ(player.level(), 1);
    EXPECT_EQ(player.populationCap(), 3);
}

TEST(PlayerTest, ThrowsForInvalidConstruction) {
    EXPECT_THROW(Player(-1, 10, 100, 1, 3), std::invalid_argument);
    EXPECT_THROW(Player(1, -1, 100, 1, 3), std::invalid_argument);
    EXPECT_THROW(Player(1, 10, 0, 1, 3), std::invalid_argument);
}

TEST(PlayerTest, AddsAndRemovesUnits) {
    Player player(1, 10, 100, 1, 3);
    player.addUnit(100);
    player.addUnit(200);
    EXPECT_TRUE(player.ownsUnit(100));
    EXPECT_EQ(player.unitCount(), 2);

    player.removeUnit(100);
    EXPECT_FALSE(player.ownsUnit(100));
    EXPECT_EQ(player.unitCount(), 1);
}

TEST(PlayerTest, PopulationOnBoardCountsOwnedUnitsOnly) {
    Board board(8, 8, 8);
    Player player(1, 10, 100, 1, 4);
    player.addUnit(10);
    player.addUnit(20);
    player.addUnit(30);

    Position a{4, 0};
    Position b{5, 1};
    Position c{6, 2};
    ASSERT_TRUE(board.placeOnBoard(10, a));
    ASSERT_TRUE(board.placeOnBoard(20, b));
    ASSERT_TRUE(board.placeOnBoard(999, c));

    EXPECT_EQ(player.populationOnBoard(board), 2);
}
