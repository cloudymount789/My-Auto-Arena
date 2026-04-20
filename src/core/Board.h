#ifndef MY_AUTO_ARENA_CORE_BOARD_H
#define MY_AUTO_ARENA_CORE_BOARD_H

#include <optional>
#include <vector>

#include "core/Tile.h"

namespace my_auto_arena::core {

struct Position {
    int row;
    int col;
};

class Board {
public:
    Board(int rows, int cols, int bench_size);

    [[nodiscard]] int rows() const noexcept;
    [[nodiscard]] int cols() const noexcept;
    [[nodiscard]] int bench_size() const noexcept;
    [[nodiscard]] bool in_bounds(Position position) const noexcept;

    bool place_on_board(int unit_id, Position position);
    bool clear_on_board(Position position);
    [[nodiscard]] std::optional<int> occupant_on_board(Position position) const;

    bool place_on_bench(int unit_id, int index);
    bool clear_on_bench(int index);
    [[nodiscard]] std::optional<int> occupant_on_bench(int index) const;

private:
    int rows_;
    int cols_;
    std::vector<Tile> tiles_;
    std::vector<int> bench_units_;

    [[nodiscard]] int tile_index(Position position) const noexcept;
};

}  // namespace my_auto_arena::core

#endif  // MY_AUTO_ARENA_CORE_BOARD_H
