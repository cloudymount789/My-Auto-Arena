#include "core/Board.h"

#include <stdexcept>

namespace my_auto_arena::core {

namespace {
int require_positive(const int value, const char* message) {
    if (value <= 0) {
        throw std::invalid_argument(message);
    }
    return value;
}
}  // namespace

Board::Board(const int rows, const int cols, const int bench_size)
    : rows_(require_positive(rows, "Rows must be positive.")),
      cols_(require_positive(cols, "Cols must be positive.")),
      bench_units_(require_positive(bench_size, "Bench size must be positive."), -1) {
    tiles_.reserve(rows_ * cols_);
    for (int row = 0; row < rows_; ++row) {
        for (int col = 0; col < cols_; ++col) {
            tiles_.emplace_back(row, col);
        }
    }
}

int Board::rows() const noexcept { return rows_; }

int Board::cols() const noexcept { return cols_; }

int Board::bench_size() const noexcept { return static_cast<int>(bench_units_.size()); }

bool Board::in_bounds(const Position position) const noexcept {
    return position.row >= 0 && position.row < rows_ && position.col >= 0 && position.col < cols_;
}

bool Board::place_on_board(const int unit_id, const Position position) {
    if (!in_bounds(position)) {
        return false;
    }
    return tiles_[tile_index(position)].place(unit_id);
}

bool Board::clear_on_board(const Position position) {
    if (!in_bounds(position)) {
        return false;
    }
    tiles_[tile_index(position)].clear();
    return true;
}

std::optional<int> Board::occupant_on_board(const Position position) const {
    if (!in_bounds(position)) {
        return std::nullopt;
    }
    const Tile& tile = tiles_[tile_index(position)];
    if (!tile.occupied()) {
        return std::nullopt;
    }
    return tile.occupant_id();
}

bool Board::place_on_bench(const int unit_id, const int index) {
    if (index < 0 || index >= bench_size() || unit_id < 0 || bench_units_[index] >= 0) {
        return false;
    }
    bench_units_[index] = unit_id;
    return true;
}

bool Board::clear_on_bench(const int index) {
    if (index < 0 || index >= bench_size()) {
        return false;
    }
    bench_units_[index] = -1;
    return true;
}

std::optional<int> Board::occupant_on_bench(const int index) const {
    if (index < 0 || index >= bench_size()) {
        return std::nullopt;
    }
    if (bench_units_[index] < 0) {
        return std::nullopt;
    }
    return bench_units_[index];
}

int Board::tile_index(const Position position) const noexcept { return position.row * cols_ + position.col; }

}  // namespace my_auto_arena::core
