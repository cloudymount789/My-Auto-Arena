#include "core/Board.h"

#include <gtest/gtest.h>
#include <stdexcept>

using my_auto_arena::core::Board;
using my_auto_arena::core::Position;

TEST(BoardTest, CreatesBoardAndBenchWithExpectedSizes) {
    const Board board(8, 8, 8);

    EXPECT_EQ(board.rows(), 8);
    EXPECT_EQ(board.cols(), 8);
    EXPECT_EQ(board.benchSize(), 8);
}

TEST(BoardTest, PlacesAndClearsBoardTile) {
    Board board(8, 8, 8);
    const Position position{2, 3};

    EXPECT_TRUE(board.placeOnBoard(1001, position));
    EXPECT_EQ(board.occupantOnBoard(position), 1001);

    EXPECT_TRUE(board.clearOnBoard(position));
    EXPECT_EQ(board.occupantOnBoard(position), -1);
}

TEST(BoardTest, RejectsInvalidBoardPlacement) {
    Board board(8, 8, 8);
    const Position out_of_bounds{-1, 3};

    EXPECT_FALSE(board.placeOnBoard(1001, out_of_bounds));
    EXPECT_FALSE(board.clearOnBoard(out_of_bounds));
}

TEST(BoardTest, PlacesAndClearsBenchSlot) {
    Board board(8, 8, 8);

    EXPECT_TRUE(board.placeOnBench(1001, 0));
    EXPECT_EQ(board.occupantOnBench(0), 1001);

    EXPECT_TRUE(board.clearOnBench(0));
    EXPECT_EQ(board.occupantOnBench(0), -1);
}

TEST(BoardTest, RejectsInvalidBenchPlacement) {
    Board board(8, 8, 8);

    EXPECT_FALSE(board.placeOnBench(1001, -1));
    EXPECT_FALSE(board.placeOnBench(1001, 999));
    EXPECT_FALSE(board.clearOnBench(-1));
}

TEST(BoardTest, RejectsDuplicateOccupancyOnBoardAndBench) {
    Board board(8, 8, 8);
    const Position position{1, 1};

    EXPECT_TRUE(board.placeOnBoard(1001, position));
    EXPECT_FALSE(board.placeOnBoard(1002, position));

    EXPECT_TRUE(board.placeOnBench(2001, 2));
    EXPECT_FALSE(board.placeOnBench(2002, 2));
}

TEST(BoardTest, ThrowsForInvalidBoardConstructionArguments) {
    EXPECT_THROW(Board(0, 8, 8), std::invalid_argument);
    EXPECT_THROW(Board(8, -1, 8), std::invalid_argument);
    EXPECT_THROW(Board(8, 8, 0), std::invalid_argument);
}

TEST(BoardTest, SplitsPlayerAndEnemyHalfByRow) {
    Board board(8, 8, 8);
    Position enemyPos{2, 3};
    Position playerPos{6, 1};
    Position outPos{99, 1};

    EXPECT_TRUE(board.isEnemyHalf(enemyPos));
    EXPECT_FALSE(board.isPlayerHalf(enemyPos));

    EXPECT_TRUE(board.isPlayerHalf(playerPos));
    EXPECT_FALSE(board.isEnemyHalf(playerPos));

    EXPECT_FALSE(board.isPlayerHalf(outPos));
    EXPECT_FALSE(board.isEnemyHalf(outPos));
}
