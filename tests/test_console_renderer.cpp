#include "ui/ConsoleRenderer.h"

#include <gtest/gtest.h>
#include <sstream>
#include <vector>

#include "core/Board.h"
#include "core/Unit.h"

using my_auto_arena::core::Board;
using my_auto_arena::core::Position;
using my_auto_arena::core::Unit;
using my_auto_arena::core::UnitOwner;
using my_auto_arena::core::WarriorUnit;
using my_auto_arena::ui::ConsoleRenderer;

TEST(ConsoleRendererTest, RendersBoardAndBenchSections) {
    Board board(8, 8, 8);
    WarriorUnit warrior(1, UnitOwner::player);
    Position position{0, 0};
    ASSERT_TRUE(board.placeOnBoard(warrior.id(), position));

    std::vector<const Unit*> units;
    units.push_back(&warrior);

    ConsoleRenderer renderer;
    std::ostringstream out;
    renderer.render(out, board, units);

    const std::string content = out.str();
    EXPECT_NE(content.find("Board (8x8)"), std::string::npos);
    EXPECT_NE(content.find("Bench"), std::string::npos);
    EXPECT_NE(content.find("[Wa]"), std::string::npos);
}

TEST(ConsoleRendererTest, RendersUnitStats) {
    Board board(8, 8, 8);
    WarriorUnit warrior(2, UnitOwner::player);

    std::vector<const Unit*> units;
    units.push_back(&warrior);

    ConsoleRenderer renderer;
    std::ostringstream out;
    renderer.render(out, board, units);

    const std::string content = out.str();
    EXPECT_NE(content.find("ID:2"), std::string::npos);
    EXPECT_NE(content.find("HP:800/800"), std::string::npos);
    EXPECT_NE(content.find("ATK:65"), std::string::npos);
    EXPECT_NE(content.find("Range:1"), std::string::npos);
}
