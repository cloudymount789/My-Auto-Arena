#ifndef MY_AUTO_ARENA_UI_QT_QT_MAIN_WINDOW_H
#define MY_AUTO_ARENA_UI_QT_QT_MAIN_WINDOW_H

#include <map>
#include <vector>

#include <QGraphicsView>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QTimer>

#include "core/BattleEngine.h"
#include "core/Board.h"
#include "core/DragDropHandler.h"
#include "core/EnemySpawner.h"
#include "core/GameFSM.h"
#include "core/HeroUnits.h"
#include "core/Player.h"
#include "core/PvERoundRunner.h"
#include "core/Unit.h"

namespace my_auto_arena {
namespace ui {

class ArenaScene;
class UnitInfoPanel;

// 主窗口：持有所有游戏状态（棋盘、玩家、单位、FSM、生成器），
// 并通过控制面板驱动 Phase 2 的 准备 → 战斗 → 结算 循环。
class QtMainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit QtMainWindow(QWidget* parent);
    virtual ~QtMainWindow() override;

private slots:
    void onUnitSelected(int unitId);
    void onDragResult(core::DragResult result);
    void onStartBattle();
    void onNextRound();
    void onBattleTick();  // 定时器每步回调：推进若干 tick 并刷新场景

private:
    // ── 游戏核心状态 ─────────────────────────────────────────────
    core::Board board_;
    core::Player player_;
    std::map<int, core::Unit*> unitsMap_;  // 玩家+敌方单位均在此注册；敌方由 doSettlement 负责释放
    core::GameFSM fsm_;
    core::EnemySpawner spawner_;
    int nextUnitId_;                       // 敌方单位 ID 分配计数器（从 100 开始）

    // ── 战斗动画状态 ─────────────────────────────────────────────
    QTimer* battleTimer_;
    core::BattleEngine* battleEngine_;         // 当前战斗引擎，空闲时为 nullptr
    std::vector<core::Unit*> spawnedEnemies_;  // 本轮生成的敌方单位（用于结算时释放）
    core::LevelConfig currentLevelCfg_;        // 本轮配置（奖励/惩罚值）
    static const int kTicksPerStep = 3;        // 每次定时器回调运行的 tick 数

    // ── Qt UI 组件 ───────────────────────────────────────────────
    ArenaScene* scene_;
    QGraphicsView* view_;
    UnitInfoPanel* infoPanel_;

    // 控制面板标签
    QLabel* phaseLabel_;
    QLabel* roundLabel_;
    QLabel* playerHpLabel_;
    QLabel* playerGoldLabel_;

    // 控制按钮
    QPushButton* startBattleBtn_;
    QPushButton* nextRoundBtn_;

    // ── 辅助方法 ─────────────────────────────────────────────────
    void updateStatusPanel();
    void doSettlement();  // 战斗结束后：结算金币/HP、清理单位、切换 FSM
};

}  // namespace ui
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_UI_QT_QT_MAIN_WINDOW_H
