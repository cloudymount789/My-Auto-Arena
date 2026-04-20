#include "core/Tile.h"

namespace my_auto_arena::core {

Tile::Tile(const int row, const int col) noexcept : row_(row), col_(col) {}

int Tile::row() const noexcept { return row_; }

int Tile::col() const noexcept { return col_; }

bool Tile::occupied() const noexcept { return occupied_; }

int Tile::occupant_id() const noexcept { return occupant_id_; }

bool Tile::place(const int unit_id) noexcept {
    if (occupied_ || unit_id < 0) {
        return false;
    }
    occupied_ = true;
    occupant_id_ = unit_id;
    return true;
}

void Tile::clear() noexcept {
    occupied_ = false;
    occupant_id_ = -1;
}

}  // namespace my_auto_arena::core
