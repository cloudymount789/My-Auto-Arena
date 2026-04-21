#include "core/Tile.h"

namespace my_auto_arena {
namespace core {

Tile::Tile(int row, int col) : row_(row), col_(col) {}

Tile::Tile(const Tile& other)
    : row_(other.row_), col_(other.col_), occupied_(other.occupied_), occupantId_(other.occupantId_) {}

int Tile::row() const { return row_; }

int Tile::col() const { return col_; }

bool Tile::occupied() const { return occupied_; }

int Tile::occupantId() const { return occupantId_; }

bool Tile::place(int unitId) {
    if (occupied_ || unitId < 0) {
        return false;
    }
    occupied_ = true;
    occupantId_ = unitId;
    return true;
}

void Tile::clear() {
    occupied_ = false;
    occupantId_ = -1;
}

}  // namespace core
}  // namespace my_auto_arena
