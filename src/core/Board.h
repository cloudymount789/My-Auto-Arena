#ifndef MY_AUTO_ARENA_CORE_BOARD_H
#define MY_AUTO_ARENA_CORE_BOARD_H

#include <vector>

#include "core/Tile.h"

namespace my_auto_arena {
namespace core {

struct Position {
    int row;
    int col;
};

class Board {
public:
    static const int kEmptySlot = -1;

    Board(int rows, int cols, int benchSize);
    Board(const Board& other);

    int rows() const;
    int cols() const;
    int benchSize() const;
    bool inBounds(Position position) const;
    bool isPlayerHalf(Position position) const;
    bool isEnemyHalf(Position position) const;

    bool placeOnBoard(int unitId, Position position);
    bool clearOnBoard(Position position);
    // 返回占用单位ID；若空位或越界返回 kEmptySlot。
    int occupantOnBoard(Position position) const;
    Position findUnitOnBoard(int unitId) const;

    bool placeOnBench(int unitId, int index);
    bool clearOnBench(int index);
    // 返回占用单位ID；若空位或越界返回 kEmptySlot。
    int occupantOnBench(int index) const;

private:
    int rows_;
    int cols_;
    std::vector<Tile> tiles_;
    std::vector<int> benchUnits_;

    int tileIndex(Position position) const;
};

}  // namespace core
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_CORE_BOARD_H
