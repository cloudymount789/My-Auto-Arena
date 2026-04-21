#include "ui/qt/TileGraphicsItem.h"

#include <QBrush>
#include <QColor>
#include <QPen>

namespace my_auto_arena {
namespace ui {

TileGraphicsItem::TileGraphicsItem(TileRegion region, int logicalRow, int logicalCol, const QRectF& rect)
    : QGraphicsRectItem(rect), region_(region), logicalRow_(logicalRow), logicalCol_(logicalCol), highlighted_(false) {
    QBrush brush(QColor("#E8E8E8"));
    if (region_ == TileRegion::kBoardPlayer) {
        brush.setColor(QColor("#D0E8FF"));
    } else if (region_ == TileRegion::kBoardEnemy) {
        brush.setColor(QColor("#FFD0D0"));
    }
    setBrush(brush);
    setPen(QPen(QColor("#666666")));
}

int TileGraphicsItem::logicalRow() const { return logicalRow_; }

int TileGraphicsItem::logicalCol() const { return logicalCol_; }

void TileGraphicsItem::setHighlighted(bool highlighted) {
    highlighted_ = highlighted;
    if (highlighted_) {
        setPen(QPen(QColor("#FFD700"), 3.0));
    } else {
        setPen(QPen(QColor("#666666")));
    }
}

}  // namespace ui
}  // namespace my_auto_arena
