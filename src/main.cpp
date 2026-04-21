#include <iostream>
#include <vector>

#include "core/Board.h"
#include "core/DragDropHandler.h"
#include "core/Unit.h"
#include "ui/ConsoleRenderer.h"

int main() {
    my_auto_arena::core::Board board(8, 8, 8);
    my_auto_arena::core::WarriorUnit warrior(1, my_auto_arena::core::UnitOwner::player);
    my_auto_arena::core::MageUnit mage(2, my_auto_arena::core::UnitOwner::player);
    my_auto_arena::core::Player player(1, 10, 100, 1, 3);
    player.addUnit(warrior.id());
    player.addUnit(mage.id());

    board.placeOnBench(warrior.id(), 0);
    board.placeOnBench(mage.id(), 1);

    my_auto_arena::core::DragDropHandler dragHandler(board, player);
    dragHandler.execute(my_auto_arena::core::DragLocation::fromBench(0),
                        my_auto_arena::core::DragLocation::fromBoard(4, 3));
    dragHandler.execute(my_auto_arena::core::DragLocation::fromBench(1),
                        my_auto_arena::core::DragLocation::fromBoard(4, 4));

    std::vector<const my_auto_arena::core::Unit*> units;
    units.push_back(&warrior);
    units.push_back(&mage);

    my_auto_arena::ui::ConsoleRenderer renderer;
    renderer.render(std::cout, board, units);
    return 0;
}
