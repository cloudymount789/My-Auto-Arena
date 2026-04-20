#ifndef MY_AUTO_ARENA_CORE_TILE_H
#define MY_AUTO_ARENA_CORE_TILE_H

namespace my_auto_arena::core {

class Tile {
public:
    Tile(int row, int col) noexcept;

    [[nodiscard]] int row() const noexcept;
    [[nodiscard]] int col() const noexcept;
    [[nodiscard]] bool occupied() const noexcept;
    [[nodiscard]] int occupant_id() const noexcept;

    bool place(int unit_id) noexcept;
    void clear() noexcept;

private:
    int row_;
    int col_;
    bool occupied_{false};
    int occupant_id_{-1};
};

}  // namespace my_auto_arena::core

#endif  // MY_AUTO_ARENA_CORE_TILE_H
