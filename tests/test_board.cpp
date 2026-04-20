#include "core/Board.h"

#include <gtest/gtest.h>
#include <stdexcept>

using my_auto_arena::core::Board;
using my_auto_arena::core::Position;

TEST(BoardTest, CreatesBoardAndBenchWithExpectedSizes) {
    const Board board(8, 8, 8);

    EXPECT_EQ(board.rows(), 8);
    EXPECT_EQ(board.cols(), 8);
    EXPECT_EQ(board.bench_size(), 8);
}

TEST(BoardTest, PlacesAndClearsBoardTile) {
    Board board(8, 8, 8);
    const Position position{2, 3};

    EXPECT_TRUE(board.place_on_board(1001, position));
    ASSERT_TRUE(board.occupant_on_board(position).has_value());
    EXPECT_EQ(board.occupant_on_board(position).value(), 1001);

    EXPECT_TRUE(board.clear_on_board(position));
    EXPECT_FALSE(board.occupant_on_board(position).has_value());
}

TEST(BoardTest, RejectsInvalidBoardPlacement) {
    Board board(8, 8, 8);
    const Position out_of_bounds{-1, 3};

    EXPECT_FALSE(board.place_on_board(1001, out_of_bounds));
    EXPECT_FALSE(board.clear_on_board(out_of_bounds));
}

TEST(BoardTest, PlacesAndClearsBenchSlot) {
    Board board(8, 8, 8);

    EXPECT_TRUE(board.place_on_bench(1001, 0));
    ASSERT_TRUE(board.occupant_on_bench(0).has_value());
    EXPECT_EQ(board.occupant_on_bench(0).value(), 1001);

    EXPECT_TRUE(board.clear_on_bench(0));
    EXPECT_FALSE(board.occupant_on_bench(0).has_value());
}

TEST(BoardTest, RejectsInvalidBenchPlacement) {
    Board board(8, 8, 8);

    EXPECT_FALSE(board.place_on_bench(1001, -1));
    EXPECT_FALSE(board.place_on_bench(1001, 999));
    EXPECT_FALSE(board.clear_on_bench(-1));
}

TEST(BoardTest, RejectsDuplicateOccupancyOnBoardAndBench) {
    Board board(8, 8, 8);
    const Position position{1, 1};

    EXPECT_TRUE(board.place_on_board(1001, position));
    EXPECT_FALSE(board.place_on_board(1002, position));

    EXPECT_TRUE(board.place_on_bench(2001, 2));
    EXPECT_FALSE(board.place_on_bench(2002, 2));
}

TEST(BoardTest, ThrowsForInvalidBoardConstructionArguments) {
    EXPECT_THROW(Board(0, 8, 8), std::invalid_argument);
    EXPECT_THROW(Board(8, -1, 8), std::invalid_argument);
    EXPECT_THROW(Board(8, 8, 0), std::invalid_argument);
}
