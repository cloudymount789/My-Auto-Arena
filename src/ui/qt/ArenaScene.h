#ifndef MY_AUTO_ARENA_UI_QT_ARENA_SCENE_H
#define MY_AUTO_ARENA_UI_QT_ARENA_SCENE_H

#include <map>
#include <vector>

#include <QGraphicsItem>
#include <QGraphicsScene>

#include "core/BattleEngine.h"
#include "core/Board.h"
#include "core/DragDropHandler.h"
#include "core/Player.h"
#include "core/Unit.h"
#include "ui/qt/SceneCoordMapper.h"

namespace my_auto_arena {
namespace ui {

class UnitGraphicsItem;
class TileGraphicsItem;

// 游戏场景：负责棋盘/备战区的图元管理与拖拽交互。
// unitsMap 由 QtMainWindow 所有，ArenaScene 只持有引用（不负责内存释放）。
class ArenaScene : public QGraphicsScene {
    Q_OBJECT
public:
    ArenaScene(core::Board& board, core::Player& player, std::map<int, core::Unit*>& unitsMap, QObject* parent);

    const core::Unit* unitById(int unitId) const;
    void rebuild();

    // 战斗开始前：将新生成的敌方单位图元加入场景。
    void addUnitItem(core::Unit* unit);

    // 战斗结算后：移除不再存在于 unitsMap 的图元，并刷新幸存单位的血蓝条。
    void syncAfterBattle(const std::map<int, core::Unit*>& unitsMap);

    // 根据本 tick 的战斗事件，在场景中生成攻击/技能特效图元（持续到下次 syncAfterBattle）。
    void spawnVfx(const std::vector<core::BattleEvent>& events);

    // 控制拖拽交互是否响应（准备阶段 true，战斗/结算阶段 false）。
    void setDragEnabled(bool enabled);

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
    std::map<int, core::Unit*>& unitsMap_;
    core::DragDropHandler dragHandler_;
    SceneCoordMapper mapper_;
    std::map<int, UnitGraphicsItem*> unitItems_;
    std::vector<TileGraphicsItem*> tileItems_;
    TileGraphicsItem* highlightedTile_;
    bool dragEnabled_;

    core::DragLocation locateUnit(int unitId) const;
    void syncUnitPositions();
    void snapBack(UnitGraphicsItem* item);
    void clearTileHighlight();
    UnitGraphicsItem* createUnitItem(core::Unit* unit);

    // 清除上一 tick 留下的所有特效图元。
    void clearVfxItems();
    // 将棋盘格子坐标转换为场景像素中心点（与 ArenaScene 构造时的布局一致）。
    QPointF tilePixelCenter(int row, int col) const;

    std::vector<QGraphicsItem*> vfxItems_;  // 临时特效图元列表
};

}  // namespace ui
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_UI_QT_ARENA_SCENE_H
