#ifndef MY_AUTO_ARENA_CORE_TILE_H
#define MY_AUTO_ARENA_CORE_TILE_H

namespace my_auto_arena {
namespace core {

class Tile {
public:
    // 单个棋盘格：只负责位置与占用状态。
    Tile(int row, int col);
    Tile(const Tile& other);

    int row() const;
    int col() const;
    bool occupied() const;
    int occupantId() const;

    bool place(int unitId);
    void clear();

private:
    int row_;
    int col_;
    bool occupied_{false};
    int occupantId_{-1};
};

}  // namespace core
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_CORE_TILE_H
