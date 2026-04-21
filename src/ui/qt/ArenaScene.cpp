#include "ui/qt/ArenaScene.h"

#include <QGraphicsRectItem>
#include <QPropertyAnimation>

#include "ui/qt/TileGraphicsItem.h"
#include "ui/qt/UnitGraphicsItem.h"

namespace my_auto_arena {
namespace ui {

ArenaScene::ArenaScene(core::Board& board, core::Player& player, const std::vector<core::Unit*>& units, QObject* parent)
    : QGraphicsScene(parent),
      board_(board),
      player_(player),
      units_(units),
      dragHandler_(board, player),
      mapper_(board.rows(), board.cols(), board.benchSize(), 64.0, 20.0, 20.0, 24.0),
      highlightedTile_(nullptr) {
    setSceneRect(0.0, 0.0, 600.0, 700.0);

    for (int row = 0; row < board.rows(); ++row) {
        for (int col = 0; col < board.cols(); ++col) {
            const QRectF rect(20.0 + col * 64.0, 20.0 + row * 64.0, 64.0, 64.0);
            TileGraphicsItem::TileRegion region =
                board.isPlayerHalf(core::Position{row, col}) ? TileGraphicsItem::TileRegion::kBoardPlayer
                                                              : TileGraphicsItem::TileRegion::kBoardEnemy;
            TileGraphicsItem* tile = new TileGraphicsItem(region, row, col, rect);
            addItem(tile);
            tileItems_.push_back(tile);
        }
    }

    const double benchY = 20.0 + board.rows() * 64.0 + 24.0;
    for (int i = 0; i < board.benchSize(); ++i) {
        const QRectF rect(20.0 + i * 64.0, benchY, 64.0, 64.0);
        TileGraphicsItem* tile = new TileGraphicsItem(TileGraphicsItem::TileRegion::kBench, -1, i, rect);
        addItem(tile);
        tileItems_.push_back(tile);
    }

    for (std::size_t i = 0; i < units_.size(); ++i) {
        core::Unit* unit = units_[i];
        UnitGraphicsItem* item =
            new UnitGraphicsItem(unit->id(), QString::fromStdString(unit->name()), unit->hp(), unit->maxHp(),
                                 unit->mana(), unit->maxMana(), 64.0);
        item->setZValue(10.0);
        connect(item, SIGNAL(dragMoved(int, QPointF)), this, SLOT(onDragMoved(int, QPointF)));
        connect(item, SIGNAL(dragFinished(int, QPointF)), this, SLOT(onDragFinished(int, QPointF)));
        connect(item, SIGNAL(unitClicked(int)), this, SLOT(onUnitClicked(int)));
        addItem(item);
        unitItems_[unit->id()] = item;
    }

    syncUnitPositions();
}

const core::Unit* ArenaScene::unitById(int unitId) const {
    for (std::size_t i = 0; i < units_.size(); ++i) {
        if (units_[i]->id() == unitId) {
            return units_[i];
        }
    }
    return nullptr;
}

void ArenaScene::rebuild() { syncUnitPositions(); }

void ArenaScene::onDragMoved(int, QPointF scenePos) {
    core::DragLocation to = core::DragLocation::fromBench(0);
    if (!mapper_.pixelToLocation(scenePos.x(), scenePos.y(), to)) {
        clearTileHighlight();
        return;
    }

    for (std::size_t i = 0; i < tileItems_.size(); ++i) {
        TileGraphicsItem* tile = tileItems_[i];
        bool hit = false;
        if (to.type == core::DragLocation::kBoard) {
            hit = (tile->logicalRow() == to.boardPos.row && tile->logicalCol() == to.boardPos.col);
        } else {
            hit = (tile->logicalRow() == -1 && tile->logicalCol() == to.benchIndex);
        }

        if (hit) {
            if (highlightedTile_ != tile) {
                clearTileHighlight();
                highlightedTile_ = tile;
                highlightedTile_->setHighlighted(true);
            }
            return;
        }
    }

    clearTileHighlight();
}

void ArenaScene::onDragFinished(int unitId, QPointF releaseScenePos) {
    std::map<int, UnitGraphicsItem*>::iterator it = unitItems_.find(unitId);
    if (it == unitItems_.end()) {
        return;
    }
    UnitGraphicsItem* item = it->second;

    core::DragLocation from = locateUnit(unitId);
    if (from.type == core::DragLocation::kBench && from.benchIndex < 0) {
        snapBack(item);
        clearTileHighlight();
        emit dragResultReady(core::DragResult::kInvalidSource);
        return;
    }
    core::DragLocation to = core::DragLocation::fromBench(0);
    if (!mapper_.pixelToLocation(releaseScenePos.x(), releaseScenePos.y(), to)) {
        snapBack(item);
        clearTileHighlight();
        emit dragResultReady(core::DragResult::kOutOfBounds);
        return;
    }

    const core::DragResult result = dragHandler_.execute(from, to);
    if (result == core::DragResult::kSuccess || result == core::DragResult::kSwapped ||
        result == core::DragResult::kSameLocation) {
        syncUnitPositions();
    } else {
        snapBack(item);
    }
    clearTileHighlight();
    emit dragResultReady(result);
}

void ArenaScene::onUnitClicked(int unitId) { emit unitSelected(unitId); }

core::DragLocation ArenaScene::locateUnit(int unitId) const {
    for (int row = 0; row < board_.rows(); ++row) {
        for (int col = 0; col < board_.cols(); ++col) {
            if (board_.occupantOnBoard(core::Position{row, col}) == unitId) {
                return core::DragLocation::fromBoard(row, col);
            }
        }
    }
    for (int i = 0; i < board_.benchSize(); ++i) {
        if (board_.occupantOnBench(i) == unitId) {
            return core::DragLocation::fromBench(i);
        }
    }
    return core::DragLocation::fromBench(-1);
}

void ArenaScene::syncUnitPositions() {
    for (std::map<int, UnitGraphicsItem*>::iterator it = unitItems_.begin(); it != unitItems_.end(); ++it) {
        const int unitId = it->first;
        UnitGraphicsItem* item = it->second;
        const core::DragLocation location = locateUnit(unitId);
        if ((location.type == core::DragLocation::kBench && location.benchIndex < 0) ||
            (location.type == core::DragLocation::kBoard && !board_.inBounds(location.boardPos))) {
            item->hide();
            continue;
        }

        double cx = 0.0;
        double cy = 0.0;
        mapper_.locationToPixelCenter(location, cx, cy);
        const QRectF br = item->boundingRect();
        item->setPos(cx - br.width() / 2.0, cy - br.height() / 2.0);
        item->show();
    }
}

void ArenaScene::snapBack(UnitGraphicsItem* item) {
    QPropertyAnimation* animation = new QPropertyAnimation(item, "pos", item);
    animation->setDuration(200);
    animation->setStartValue(item->pos());
    animation->setEndValue(item->dragStartScenePos());
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void ArenaScene::clearTileHighlight() {
    if (highlightedTile_ != nullptr) {
        highlightedTile_->setHighlighted(false);
        highlightedTile_ = nullptr;
    }
}

}  // namespace ui
}  // namespace my_auto_arena
