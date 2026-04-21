#include "core/Pathfinder.h"

#include <gtest/gtest.h>

#include <map>

#include "core/Board.h"
#include "core/Unit.h"

using my_auto_arena::core::Board;
using my_auto_arena::core::Pathfinder;
using my_auto_arena::core::Position;
using my_auto_arena::core::UnitOwner;
using my_auto_arena::core::WarriorUnit;

TEST(PathfinderTest, FindsNextStepTowardAttackRange) {
    Board board(5, 5, 1);
    WarriorUnit mover(1, UnitOwner::player);
    WarriorUnit enemy(2, UnitOwner::enemy);
    std::map<int, my_auto_arena::core::Unit*> reg;
    reg[mover.id()] = &mover;
    reg[enemy.id()] = &enemy;
    ASSERT_TRUE(board.placeOnBoard(mover.id(), Position{4, 4}));
    ASSERT_TRUE(board.placeOnBoard(enemy.id(), Position{0, 0}));

    Position next;
    ASSERT_TRUE(Pathfinder::nextStepTowardAttackRange(board, reg, mover.id(), Position{4, 4}, Position{0, 0}, 1, &next));
    const int dr = next.row - 4;
    const int dc = next.col - 4;
    EXPECT_TRUE((dr == -1 && dc == 0) || (dr == 0 && dc == -1));
}

TEST(PathfinderTest, ReturnsFalseWhenNoEmptyCellInAttackRange) {
    Board board(2, 2, 1);
    WarriorUnit mover(1, UnitOwner::player);
    WarriorUnit blockA(2, UnitOwner::enemy);
    WarriorUnit blockB(3, UnitOwner::enemy);
    WarriorUnit target(4, UnitOwner::enemy);
    std::map<int, my_auto_arena::core::Unit*> reg;
    reg[mover.id()] = &mover;
    reg[blockA.id()] = &blockA;
    reg[blockB.id()] = &blockB;
    reg[target.id()] = &target;
    ASSERT_TRUE(board.placeOnBoard(target.id(), Position{0, 0}));
    ASSERT_TRUE(board.placeOnBoard(blockA.id(), Position{0, 1}));
    ASSERT_TRUE(board.placeOnBoard(blockB.id(), Position{1, 0}));
    ASSERT_TRUE(board.placeOnBoard(mover.id(), Position{1, 1}));

    Position next;
    EXPECT_FALSE(Pathfinder::nextStepTowardAttackRange(board, reg, mover.id(), Position{1, 1}, Position{0, 0}, 1, &next));
}
