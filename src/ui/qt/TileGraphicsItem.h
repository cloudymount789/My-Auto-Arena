#ifndef MY_AUTO_ARENA_UI_QT_TILE_GRAPHICS_ITEM_H
#define MY_AUTO_ARENA_UI_QT_TILE_GRAPHICS_ITEM_H

#include <QGraphicsRectItem>

namespace my_auto_arena {
namespace ui {

class TileGraphicsItem : public QGraphicsRectItem {
public:
    enum class TileRegion { kBoardPlayer, kBoardEnemy, kBench };

    TileGraphicsItem(TileRegion region, int logicalRow, int logicalCol, const QRectF& rect);

    int logicalRow() const;
    int logicalCol() const;
    void setHighlighted(bool highlighted);

private:
    TileRegion region_;
    int logicalRow_;
    int logicalCol_;
    bool highlighted_;
};

}  // namespace ui
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_UI_QT_TILE_GRAPHICS_ITEM_H
