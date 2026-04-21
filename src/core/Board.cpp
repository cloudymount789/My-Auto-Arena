#include "core/Board.h"

#include <stdexcept>

namespace my_auto_arena {
namespace core {

namespace {
int requirePositive(int value, const char* message) {
    if (value <= 0) {
        throw std::invalid_argument(message);
    }
    return value;
}
}  // namespace

Board::Board(int rows, int cols, int benchSize)
    : rows_(requirePositive(rows, "Rows must be positive.")),
      cols_(requirePositive(cols, "Cols must be positive.")),
      benchUnits_(requirePositive(benchSize, "Bench size must be positive."), -1) {
    tiles_.reserve(rows_ * cols_);
    for (int row = 0; row < rows_; ++row) {
        for (int col = 0; col < cols_; ++col) {
            tiles_.emplace_back(row, col);
        }
    }
}

Board::Board(const Board& other)
    : rows_(other.rows_), cols_(other.cols_), tiles_(other.tiles_), benchUnits_(other.benchUnits_) {}

int Board::rows() const { return rows_; }

int Board::cols() const { return cols_; }

int Board::benchSize() const { return static_cast<int>(benchUnits_.size()); }

bool Board::inBounds(Position position) const {
    return position.row >= 0 && position.row < rows_ && position.col >= 0 && position.col < cols_;
}

bool Board::isPlayerHalf(Position position) const {
    if (!inBounds(position)) {
        return false;
    }
    return position.row >= rows_ / 2;
}

bool Board::isEnemyHalf(Position position) const {
    if (!inBounds(position)) {
        return false;
    }
    return position.row < rows_ / 2;
}

bool Board::placeOnBoard(int unitId, Position position) {
    if (!inBounds(position)) {
        return false;
    }
    return tiles_.at(tileIndex(position)).place(unitId);
}

bool Board::clearOnBoard(Position position) {
    if (!inBounds(position)) {
        return false;
    }
    tiles_.at(tileIndex(position)).clear();
    return true;
}

int Board::occupantOnBoard(Position position) const {
    if (!inBounds(position)) {
        return kEmptySlot;
    }
    const Tile& tile = tiles_.at(tileIndex(position));
    if (!tile.occupied()) {
        return kEmptySlot;
    }
    return tile.occupantId();
}

Position Board::findUnitOnBoard(int unitId) const {
    for (int row = 0; row < rows_; ++row) {
        for (int col = 0; col < cols_; ++col) {
            Position position{row, col};
            if (occupantOnBoard(position) == unitId) {
                return position;
            }
        }
    }
    return Position{-1, -1};
}

bool Board::placeOnBench(int unitId, int index) {
    if (index < 0 || index >= benchSize() || unitId < 0 || benchUnits_.at(index) >= 0) {
        return false;
    }
    benchUnits_.at(index) = unitId;
    return true;
}

bool Board::clearOnBench(int index) {
    if (index < 0 || index >= benchSize()) {
        return false;
    }
    benchUnits_.at(index) = -1;
    return true;
}

int Board::occupantOnBench(int index) const {
    if (index < 0 || index >= benchSize()) {
        return kEmptySlot;
    }
    if (benchUnits_.at(index) < 0) {
        return kEmptySlot;
    }
    return benchUnits_.at(index);
}

int Board::tileIndex(Position position) const { return position.row * cols_ + position.col; }

}  // namespace core
}  // namespace my_auto_arena
