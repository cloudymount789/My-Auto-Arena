#ifndef MY_AUTO_ARENA_UI_QT_ARENA_SCENE_H
#define MY_AUTO_ARENA_UI_QT_ARENA_SCENE_H

#include <map>
#include <vector>

#include <QColor>
#include <QGraphicsEllipseItem>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QPointF>
#include <QTimer>

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

    // 根据本 tick 的战斗事件，在场景中生成攻击/技能特效图元。
    // 近战/静态特效持续到下次 syncAfterBattle()；飞行物通过内部定时器实时运动。
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

    // 清除所有特效图元（静态+动态），停止视觉特效定时器。
    void clearVfxItems();
    // 将棋盘格子坐标转换为场景像素中心点（与 ArenaScene 构造时的布局一致）。
    QPointF tilePixelCenter(int row, int col) const;
    // 在指定位置生成一个向外扩散并淡出的光环脉冲。
    void spawnPulse(QPointF center, QColor color, double startR, double endR, int duration);

    // ── 视觉特效动画系统 ─────────────────────────────────────────────────────────
    // 飞行物状态结构体：存储一个正在运动的抛射体的全部状态。
    struct VfxProjectile {
        QGraphicsEllipseItem* item;  // 表示飞行物的椭圆图元
        QPointF startPos;
        QPointF endPos;
        int     elapsed;    // 已经过的毫秒数
        int     duration;   // 总飞行时间（ms）
        double  radius;     // 圆半径（像素）
        double  widthScale; // 宽度缩放（>1 = 横向拉伸，模拟箭矢）
        double  angle;      // 旋转角度（度），0 = 水平向右
        // 命中后是否产生脉冲环特效（技能命中时为 true）
        bool    spawnImpactPulse;
        QColor  impactColor;
    };

    // 扩散光环结构体：技能命中或范围伤害时产生，从中心向外扩张并淡出。
    struct VfxPulse {
        QGraphicsEllipseItem* item;
        QPointF center;
        int     elapsed;
        int     duration;   // 总持续时间（ms）
        double  startRadius;
        double  endRadius;
        QColor  color;
        int     startAlpha;
    };

    std::vector<QGraphicsItem*> vfxItems_;           // 静态特效图元（每 tick 清除）
    std::vector<VfxProjectile>  activeProjectiles_;  // 动态飞行物列表
    std::vector<VfxPulse>       activePulses_;       // 动态扩散脉冲列表
    QTimer*                     vfxTimer_;           // 驱动飞行物/脉冲动画的定时器（30 ms）

private slots:
    void onVfxTick();  // vfxTimer_ 每 30 毫秒推进一次所有飞行物位置
};

}  // namespace ui
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_UI_QT_ARENA_SCENE_H
