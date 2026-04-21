#ifndef MY_AUTO_ARENA_UI_QT_ARENA_SCENE_H
#define MY_AUTO_ARENA_UI_QT_ARENA_SCENE_H

#include <map>
#include <vector>

#include <QGraphicsScene>

#include "core/Board.h"
#include "core/DragDropHandler.h"
#include "core/Player.h"
#include "core/Unit.h"
#include "ui/qt/SceneCoordMapper.h"

namespace my_auto_arena {
namespace ui {

class UnitGraphicsItem;
class TileGraphicsItem;

class ArenaScene : public QGraphicsScene {
    Q_OBJECT
public:
    ArenaScene(core::Board& board, core::Player& player, const std::vector<core::Unit*>& units, QObject* parent);

    const core::Unit* unitById(int unitId) const;
    void rebuild();

signals:
    void unitSelected(int unitId);
    void dragResultReady(core::DragResult result);

private slots:
    void onDragMoved(int unitId, QPointF scenePos);
    void onDragFinished(int unitId, QPointF releaseScenePos);
    void onUnitClicked(int unitId);

private:
    core::Board& board_;
    core::Player& player_;
    const std::vector<core::Unit*>& units_;
    core::DragDropHandler dragHandler_;
    SceneCoordMapper mapper_;
    std::map<int, UnitGraphicsItem*> unitItems_;
    std::vector<TileGraphicsItem*> tileItems_;
    TileGraphicsItem* highlightedTile_;

    core::DragLocation locateUnit(int unitId) const;
    void syncUnitPositions();
    void snapBack(UnitGraphicsItem* item);
    void clearTileHighlight();
};

}  // namespace ui
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_UI_QT_ARENA_SCENE_H
