#include "ui/ConsoleRenderer.h"

#include <iomanip>
#include <ostream>
#include <string>

namespace my_auto_arena {
namespace ui {

ConsoleRenderer::ConsoleRenderer() {}

void ConsoleRenderer::render(std::ostream& out, const core::Board& board,
                             const std::vector<const core::Unit*>& units) const {
    renderBoard(out, board, units);
    renderBench(out, board, units);
    renderUnitStats(out, units);
}

void ConsoleRenderer::renderBoard(std::ostream& out, const core::Board& board,
                                  const std::vector<const core::Unit*>& units) const {
    out << "========== Board (" << board.rows() << "x" << board.cols() << ") ==========\n";
    out << "   ";
    for (int col = 0; col < board.cols(); ++col) {
        out << std::setw(3) << col;
    }
    out << '\n';

    for (int row = 0; row < board.rows(); ++row) {
        out << std::setw(2) << row << " ";
        for (int col = 0; col < board.cols(); ++col) {
            core::Position position;
            position.row = row;
            position.col = col;
            const int unitId = board.occupantOnBoard(position);
            const core::Unit* unit = findUnitById(unitId, units);
            out << "[" << unitCode(unit) << "]";
        }
        out << '\n';
    }
}

void ConsoleRenderer::renderBench(std::ostream& out, const core::Board& board,
                                  const std::vector<const core::Unit*>& units) const {
    out << "========== Bench (" << board.benchSize() << " slots) ==========\n";
    for (int index = 0; index < board.benchSize(); ++index) {
        const int unitId = board.occupantOnBench(index);
        const core::Unit* unit = findUnitById(unitId, units);
        out << "[" << unitCode(unit) << "]";
    }
    out << '\n';
}

void ConsoleRenderer::renderUnitStats(std::ostream& out, const std::vector<const core::Unit*>& units) const {
    out << "========== Unit Stats ==========\n";
    for (int i = 0; i < static_cast<int>(units.size()); ++i) {
        const core::Unit* unit = units.at(i);
        out << "ID:" << unit->id() << "  " << unit->name() << "  HP:" << unit->hp() << "/" << unit->maxHp()
            << "  Mana:" << unit->mana() << "/" << unit->maxMana() << "  ATK:" << unit->attack()
            << "  Range:" << unit->attackRange() << '\n';
    }
}

const core::Unit* ConsoleRenderer::findUnitById(int id, const std::vector<const core::Unit*>& units) const {
    if (id < 0) {
        return nullptr;
    }
    for (int i = 0; i < static_cast<int>(units.size()); ++i) {
        const core::Unit* unit = units.at(i);
        if (unit != nullptr && unit->id() == id) {
            return unit;
        }
    }
    return nullptr;
}

std::string ConsoleRenderer::unitCode(const core::Unit* unit) const {
    if (unit == nullptr) {
        return "  ";
    }
    const std::string& name = unit->name();
    if (name.size() >= 2) {
        return name.substr(0, 2);
    }
    if (name.size() == 1) {
        return name + " ";
    }
    return "??";
}

}  // namespace ui
}  // namespace my_auto_arena
