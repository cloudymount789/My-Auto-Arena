#include "core/DragDropHandler.h"
#include "core/Player.h"

#include <gtest/gtest.h>

using my_auto_arena::core::Board;
using my_auto_arena::core::DragDropHandler;
using my_auto_arena::core::DragLocation;
using my_auto_arena::core::DragResult;
using my_auto_arena::core::Player;
using my_auto_arena::core::Position;

TEST(DragDropTest, BenchToBoardEmptySlot) {
    Board board(8, 8, 8);
    DragDropHandler handler(board);
    Position target{2, 3};
    ASSERT_TRUE(board.placeOnBench(1001, 0));

    const DragResult result = handler.execute(DragLocation::fromBench(0), DragLocation::fromBoard(2, 3));
    EXPECT_EQ(result, DragResult::kSuccess);
    EXPECT_EQ(board.occupantOnBench(0), -1);
    EXPECT_EQ(board.occupantOnBoard(target), 1001);
}

TEST(DragDropTest, BoardToBenchEmptySlot) {
    Board board(8, 8, 8);
    DragDropHandler handler(board);
    Position from{1, 1};
    ASSERT_TRUE(board.placeOnBoard(1002, from));

    const DragResult result = handler.execute(DragLocation::fromBoard(1, 1), DragLocation::fromBench(2));
    EXPECT_EQ(result, DragResult::kSuccess);
    EXPECT_EQ(board.occupantOnBoard(from), -1);
    EXPECT_EQ(board.occupantOnBench(2), 1002);
}

TEST(DragDropTest, SwapBoardToBoard) {
    Board board(8, 8, 8);
    DragDropHandler handler(board);
    Position a{0, 0};
    Position b{0, 1};
    ASSERT_TRUE(board.placeOnBoard(101, a));
    ASSERT_TRUE(board.placeOnBoard(202, b));

    const DragResult result = handler.execute(DragLocation::fromBoard(0, 0), DragLocation::fromBoard(0, 1));
    EXPECT_EQ(result, DragResult::kSwapped);
    EXPECT_EQ(board.occupantOnBoard(a), 202);
    EXPECT_EQ(board.occupantOnBoard(b), 101);
}

TEST(DragDropTest, SwapBenchToBoard) {
    Board board(8, 8, 8);
    DragDropHandler handler(board);
    Position boardPos{3, 3};
    ASSERT_TRUE(board.placeOnBench(10, 0));
    ASSERT_TRUE(board.placeOnBoard(20, boardPos));

    const DragResult result = handler.execute(DragLocation::fromBench(0), DragLocation::fromBoard(3, 3));
    EXPECT_EQ(result, DragResult::kSwapped);
    EXPECT_EQ(board.occupantOnBoard(boardPos), 10);
    EXPECT_EQ(board.occupantOnBench(0), 20);
}

TEST(DragDropTest, InvalidSourceReturnsBounceResult) {
    Board board(8, 8, 8);
    DragDropHandler handler(board);

    const DragResult result = handler.execute(DragLocation::fromBench(7), DragLocation::fromBoard(0, 0));
    EXPECT_EQ(result, DragResult::kInvalidSource);
}

TEST(DragDropTest, OutOfBoundsReturnsBounceResult) {
    Board board(8, 8, 8);
    DragDropHandler handler(board);
    ASSERT_TRUE(board.placeOnBench(9, 0));

    const DragResult result = handler.execute(DragLocation::fromBench(0), DragLocation::fromBoard(99, 99));
    EXPECT_EQ(result, DragResult::kOutOfBounds);
    EXPECT_EQ(board.occupantOnBench(0), 9);
}

TEST(DragDropTest, SameLocationIsNoOp) {
    Board board(8, 8, 8);
    DragDropHandler handler(board);
    Position position{2, 2};
    ASSERT_TRUE(board.placeOnBoard(8, position));

    const DragResult result = handler.execute(DragLocation::fromBoard(2, 2), DragLocation::fromBoard(2, 2));
    EXPECT_EQ(result, DragResult::kSameLocation);
    EXPECT_EQ(board.occupantOnBoard(position), 8);
}

TEST(DragDropTest, RejectsPlacementToEnemyHalfWhenPlayerConstraintsEnabled) {
    Board board(8, 8, 8);
    Player player(1, 10, 100, 1, 3);
    player.addUnit(1001);
    ASSERT_TRUE(board.placeOnBench(1001, 0));

    DragDropHandler handler(board, player);
    const DragResult result = handler.execute(DragLocation::fromBench(0), DragLocation::fromBoard(1, 2));
    EXPECT_EQ(result, DragResult::kNotPlayerHalf);
    EXPECT_EQ(board.occupantOnBench(0), 1001);
}

TEST(DragDropTest, RejectsPlacementWhenPopulationFull) {
    Board board(8, 8, 8);
    Player player(1, 10, 100, 1, 1);
    player.addUnit(10);
    player.addUnit(20);
    Position existing{4, 0};
    ASSERT_TRUE(board.placeOnBoard(10, existing));
    ASSERT_TRUE(board.placeOnBench(20, 0));

    DragDropHandler handler(board, player);
    const DragResult result = handler.execute(DragLocation::fromBench(0), DragLocation::fromBoard(4, 1));
    EXPECT_EQ(result, DragResult::kPopulationFull);
    EXPECT_EQ(board.occupantOnBench(0), 20);
}

TEST(DragDropTest, RejectsBoardToBoardMoveIntoEnemyHalfWhenConstraintsEnabled) {
    Board board(8, 8, 8);
    Player player(1, 10, 100, 1, 3);
    player.addUnit(42);
    Position from{4, 0};
    ASSERT_TRUE(board.placeOnBoard(42, from));

    DragDropHandler handler(board, player);
    const DragResult result = handler.execute(DragLocation::fromBoard(4, 0), DragLocation::fromBoard(1, 1));
    EXPECT_EQ(result, DragResult::kNotPlayerHalf);
    EXPECT_EQ(board.occupantOnBoard(from), 42);
}
