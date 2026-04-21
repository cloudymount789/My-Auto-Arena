#ifndef MY_AUTO_ARENA_UI_CONSOLE_RENDERER_H
#define MY_AUTO_ARENA_UI_CONSOLE_RENDERER_H

#include <iosfwd>
#include <string>
#include <vector>

#include "core/Board.h"
#include "core/Unit.h"

namespace my_auto_arena {
namespace ui {

class ConsoleRenderer {
public:
    ConsoleRenderer();

    void render(std::ostream& out, const core::Board& board, const std::vector<const core::Unit*>& units) const;

private:
    void renderBoard(std::ostream& out, const core::Board& board, const std::vector<const core::Unit*>& units) const;
    void renderBench(std::ostream& out, const core::Board& board, const std::vector<const core::Unit*>& units) const;
    void renderUnitStats(std::ostream& out, const std::vector<const core::Unit*>& units) const;
    const core::Unit* findUnitById(int id, const std::vector<const core::Unit*>& units) const;
    std::string unitCode(const core::Unit* unit) const;
};

}  // namespace ui
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_UI_CONSOLE_RENDERER_H
