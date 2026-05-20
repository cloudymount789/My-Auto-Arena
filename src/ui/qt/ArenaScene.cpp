#include "ui/qt/ArenaScene.h"

#include <QBrush>
#include <QColor>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QPen>
#include <QPropertyAnimation>

#include "ui/qt/TileGraphicsItem.h"
#include "ui/qt/UnitGraphicsItem.h"

namespace my_auto_arena {
namespace ui {

ArenaScene::ArenaScene(core::Board& board, core::Player& player, std::map<int, core::Unit*>& unitsMap,
                       QObject* parent)
    : QGraphicsScene(parent),
      board_(board),
      player_(player),
      unitsMap_(unitsMap),
      dragHandler_(board, player),
      mapper_(board.rows(), board.cols(), board.benchSize(), 64.0, 20.0, 20.0, 24.0),
      highlightedTile_(nullptr),
      dragEnabled_(true) {
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

    for (std::map<int, core::Unit*>::iterator it = unitsMap_.begin(); it != unitsMap_.end(); ++it) {
        createUnitItem(it->second);
    }

    syncUnitPositions();
}

UnitGraphicsItem* ArenaScene::createUnitItem(core::Unit* unit) {
    const bool isEnemy = (unit->owner() == core::UnitOwner::enemy);
    UnitGraphicsItem* item =
        new UnitGraphicsItem(unit->id(), QString::fromStdString(unit->name()), unit->hp(), unit->maxHp(),
                             unit->mana(), unit->maxMana(), 64.0, isEnemy, unit->starLevel());
    item->setZValue(10.0);
    connect(item, SIGNAL(dragMoved(int, QPointF)), this, SLOT(onDragMoved(int, QPointF)));
    connect(item, SIGNAL(dragFinished(int, QPointF)), this, SLOT(onDragFinished(int, QPointF)));
    connect(item, SIGNAL(unitClicked(int)), this, SLOT(onUnitClicked(int)));
    addItem(item);
    unitItems_[unit->id()] = item;
    return item;
}

void ArenaScene::addUnitItem(core::Unit* unit) {
    if (unitItems_.find(unit->id()) != unitItems_.end()) {
        return;
    }
    UnitGraphicsItem* item = createUnitItem(unit);
    double cx = 0.0;
    double cy = 0.0;
    const core::DragLocation loc = locateUnit(unit->id());
    mapper_.locationToPixelCenter(loc, cx, cy);
    const QRectF br = item->boundingRect();
    item->setPos(cx - br.width() / 2.0, cy - br.height() / 2.0);
    item->show();
}

void ArenaScene::clearVfxItems() {
    for (std::size_t i = 0; i < vfxItems_.size(); ++i) {
        removeItem(vfxItems_.at(i));
        delete vfxItems_.at(i);
    }
    vfxItems_.clear();
}

QPointF ArenaScene::tilePixelCenter(int row, int col) const {
    // 布局与构造函数一致：左边距 20px，格子 64px。
    const double cx = 20.0 + col * 64.0 + 32.0;
    const double cy = 20.0 + row * 64.0 + 32.0;
    return QPointF(cx, cy);
}

void ArenaScene::spawnVfx(const std::vector<core::BattleEvent>& events) {
    for (std::size_t i = 0; i < events.size(); ++i) {
        const core::BattleEvent& ev = events.at(i);

        // 坐标校验：源或目标不在棋盘范围内则跳过。
        const bool srcValid = (ev.srcRow >= 0 && ev.srcCol >= 0);
        const bool tgtValid = (ev.tgtRow >= 0 && ev.tgtCol >= 0);
        if (!srcValid || !tgtValid) {
            continue;
        }

        const QPointF srcCenter = tilePixelCenter(ev.srcRow, ev.srcCol);
        const QPointF tgtCenter = tilePixelCenter(ev.tgtRow, ev.tgtCol);

        if (ev.type == core::BattleEvent::Type::kAttack) {
            if (ev.isMelee) {
                // 近战攻击：目标格上橙色冲击环。
                const double r = 22.0;
                QGraphicsEllipseItem* ring =
                    new QGraphicsEllipseItem(tgtCenter.x() - r, tgtCenter.y() - r, r * 2, r * 2);
                ring->setBrush(Qt::NoBrush);
                ring->setPen(QPen(QColor(255, 130, 40, 210), 3));
                ring->setZValue(20.0);
                addItem(ring);
                vfxItems_.push_back(ring);

                // 近战命中：目标格小红点闪光
                const double dotR = 8.0;
                QGraphicsEllipseItem* dot =
                    new QGraphicsEllipseItem(tgtCenter.x() - dotR, tgtCenter.y() - dotR, dotR * 2, dotR * 2);
                dot->setBrush(QBrush(QColor(255, 60, 60, 180)));
                dot->setPen(Qt::NoPen);
                dot->setZValue(21.0);
                addItem(dot);
                vfxItems_.push_back(dot);
            } else {
                // 远程攻击：在攻击者→目标的 1/3 和 2/3 处各画一个黄色子弹点，模拟弹道。
                const QPointF p1 = srcCenter * (2.0 / 3.0) + tgtCenter * (1.0 / 3.0);
                const QPointF p2 = srcCenter * (1.0 / 3.0) + tgtCenter * (2.0 / 3.0);
                const double bR = 6.0;

                for (int pi = 0; pi < 2; ++pi) {
                    const QPointF& pt = (pi == 0) ? p1 : p2;
                    QGraphicsEllipseItem* bullet =
                        new QGraphicsEllipseItem(pt.x() - bR, pt.y() - bR, bR * 2, bR * 2);
                    bullet->setBrush(QBrush(QColor(255, 235, 60, 220)));
                    bullet->setPen(QPen(QColor(255, 200, 0, 180), 1));
                    bullet->setZValue(21.0);
                    addItem(bullet);
                    vfxItems_.push_back(bullet);
                }

                // 目标命中闪光
                const double hitR = 10.0;
                QGraphicsEllipseItem* hit =
                    new QGraphicsEllipseItem(tgtCenter.x() - hitR, tgtCenter.y() - hitR, hitR * 2, hitR * 2);
                hit->setBrush(QBrush(QColor(255, 235, 60, 140)));
                hit->setPen(Qt::NoPen);
                hit->setZValue(20.0);
                addItem(hit);
                vfxItems_.push_back(hit);
            }
        } else if (ev.type == core::BattleEvent::Type::kSkill) {
            // 技能施法：施法者格子紫色光晕 + 目标格射线。
            const double glowR = 30.0;
            QGraphicsEllipseItem* glow =
                new QGraphicsEllipseItem(srcCenter.x() - glowR, srcCenter.y() - glowR, glowR * 2, glowR * 2);
            glow->setBrush(QBrush(QColor(180, 80, 255, 90)));
            glow->setPen(QPen(QColor(200, 120, 255, 200), 2));
            glow->setZValue(20.0);
            addItem(glow);
            vfxItems_.push_back(glow);

            // 施法连线（法术弹道）
            QGraphicsLineItem* line = new QGraphicsLineItem(
                srcCenter.x(), srcCenter.y(), tgtCenter.x(), tgtCenter.y());
            line->setPen(QPen(QColor(200, 120, 255, 160), 2, Qt::DashLine));
            line->setZValue(20.0);
            addItem(line);
            vfxItems_.push_back(line);

            // 目标处技能爆炸圆圈
            const double blastR = 28.0;
            QGraphicsEllipseItem* blast =
                new QGraphicsEllipseItem(tgtCenter.x() - blastR, tgtCenter.y() - blastR, blastR * 2, blastR * 2);
            blast->setBrush(QBrush(QColor(200, 80, 255, 80)));
            blast->setPen(QPen(QColor(220, 140, 255, 200), 2));
            blast->setZValue(20.0);
            addItem(blast);
            vfxItems_.push_back(blast);
        }
    }
}

void ArenaScene::syncAfterBattle(const std::map<int, core::Unit*>& unitsMap) {
    clearVfxItems();  // 先清除上一 tick 的特效
    // 移除不再存在于 unitsMap 的图元（战斗中阵亡的单位）。
    std::vector<int> toRemove;
    for (std::map<int, UnitGraphicsItem*>::iterator it = unitItems_.begin(); it != unitItems_.end(); ++it) {
        if (unitsMap.find(it->first) == unitsMap.end()) {
            toRemove.push_back(it->first);
        }
    }
    for (std::size_t i = 0; i < toRemove.size(); ++i) {
        std::map<int, UnitGraphicsItem*>::iterator it = unitItems_.find(toRemove.at(i));
        if (it != unitItems_.end()) {
            removeItem(it->second);
            delete it->second;
            unitItems_.erase(it);
        }
    }
    // 刷新幸存单位的血蓝条和位置。
    for (std::map<int, core::Unit*>::const_iterator it = unitsMap.begin(); it != unitsMap.end(); ++it) {
        const core::Unit* unit = it->second;
        std::map<int, UnitGraphicsItem*>::iterator itemIt = unitItems_.find(unit->id());
        if (itemIt != unitItems_.end()) {
            itemIt->second->setStats(unit->hp(), unit->maxHp(), unit->mana(), unit->maxMana());
        }
    }
    syncUnitPositions();
}

void ArenaScene::setDragEnabled(bool enabled) { dragEnabled_ = enabled; }

const core::Unit* ArenaScene::unitById(int unitId) const {
    std::map<int, core::Unit*>::const_iterator it = unitsMap_.find(unitId);
    if (it != unitsMap_.end()) {
        return it->second;
    }
    return nullptr;
}

void ArenaScene::rebuild() { syncUnitPositions(); }

void ArenaScene::onDragMoved(int, QPointF scenePos) {
    if (!dragEnabled_) {
        return;
    }
    core::DragLocation to = core::DragLocation::fromBench(0);
    if (!mapper_.pixelToLocation(scenePos.x(), scenePos.y(), to)) {
        clearTileHighlight();
        return;
    }

    for (std::size_t i = 0; i < tileItems_.size(); ++i) {
        TileGraphicsItem* tile = tileItems_.at(i);
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
    if (!dragEnabled_) {
        std::map<int, UnitGraphicsItem*>::iterator it = unitItems_.find(unitId);
        if (it != unitItems_.end()) {
            snapBack(it->second);
        }
        clearTileHighlight();
        return;
    }

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
