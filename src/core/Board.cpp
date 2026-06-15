#include "core/Board.h"

#include <stdexcept>

namespace my_auto_arena {
namespace core {

namespace {
// 流程：校验构造参数必须为正数 ──> 非法抛异常 ──> 合法则原值返回
int requirePositive(int value, const char* message) {
    if (value <= 0) {
        throw std::invalid_argument(message);
    }
    return value;
}
}  // namespace

// 流程：校验尺寸 ──> 按 row×col 创建 Tile 网格 ──> 初始化备战区槽位
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

// 流程：检查坐标在棋盘内 ──> 行号位于下半区则视为玩家半场 ──> 越界直接 false
bool Board::isPlayerHalf(Position position) const {
    if (!inBounds(position)) {
        return false;
    }
    return position.row >= rows_ / 2;
}

// 流程：检查坐标在棋盘内 ──> 行号位于上半区则视为敌方半场 ──> 越界直接 false
bool Board::isEnemyHalf(Position position) const {
    if (!inBounds(position)) {
        return false;
    }
    return position.row < rows_ / 2;
}

// 流程：校验坐标 ──> 交给 Tile 写入占用单位 ID ──> 返回是否放置成功
bool Board::placeOnBoard(int unitId, Position position) {
    if (!inBounds(position)) {
        return false;
    }
    return tiles_.at(tileIndex(position)).place(unitId);
}

// 流程：校验坐标 ──> 清空对应 Tile 占用状态 ──> 越界时返回 false
bool Board::clearOnBoard(Position position) {
    if (!inBounds(position)) {
        return false;
    }
    tiles_.at(tileIndex(position)).clear();
    return true;
}

// 流程：校验坐标 ──> 越界或空格返回空槽标记 ──> 否则返回 Tile 占用单位 ID
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

// 流程：双层循环扫描棋盘 ──> 匹配 unitId ──> 返回坐标（未找到则 {-1,-1}）
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

// 流程：校验备战区下标/单位 ID/空位 ──> 写入单位 ID ──> 返回是否放置成功
bool Board::placeOnBench(int unitId, int index) {
    if (index < 0 || index >= benchSize() || unitId < 0 || benchUnits_.at(index) >= 0) {
        return false;
    }
    benchUnits_.at(index) = unitId;
    return true;
}

// 流程：校验备战区下标 ──> 清空槽位 ──> 越界时返回 false
bool Board::clearOnBench(int index) {
    if (index < 0 || index >= benchSize()) {
        return false;
    }
    benchUnits_.at(index) = -1;
    return true;
}

// 流程：校验备战区下标 ──> 越界或空槽返回空槽标记 ──> 否则返回槽位单位 ID
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
