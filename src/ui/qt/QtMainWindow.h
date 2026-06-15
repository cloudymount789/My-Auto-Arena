#ifndef MY_AUTO_ARENA_UI_QT_QT_MAIN_WINDOW_H
#define MY_AUTO_ARENA_UI_QT_QT_MAIN_WINDOW_H

#include <map>
#include <vector>

#include <QFileDialog>
#include <QGraphicsView>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

#include "core/BattleEngine.h"
#include "core/Board.h"
#include "core/DragDropHandler.h"
#include "core/EconomySystem.h"
#include "core/EnemySpawner.h"
#include "core/GameFSM.h"
#include "core/HeroUnits.h"
#include "core/Item.h"
#include "core/Player.h"
#include "core/PvERoundRunner.h"
#include "core/SaveManager.h"
#include "core/Shop.h"
#include "core/StarUpgrade.h"
#include "core/SynergySystem.h"
#include "core/Unit.h"

namespace my_auto_arena {
namespace ui {

class ArenaScene;
class ShopPanel;
class UnitInfoPanel;

// 主窗口：持有所有游戏状态（棋盘、玩家、单位、FSM、商店、羁绊等），
// 并通过控制面板驱动第三阶段的完整游戏循环。
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

    // 第三阶段新增槽函数
    void onHeroPurchased(int slotIndex);
    void onShopRefresh();
    void onSellUnit(int unitId);
    void onLevelUp();
    void onDeployFromLast();
    void onSaveGame();
    void onLoadGame();
    void onEquipItem();    // 装备待装备道具到当前选中英雄
    void onUnequipItem(int unitId, int slotIndex);  // 卸下指定槽位装备并归还 pendingItems_

private:
    // ── 游戏核心状态 ─────────────────────────────────────────────
    core::Board board_;
    core::Player player_;
    std::map<int, core::Unit*> unitsMap_;  // 玩家+敌方单位均在此注册；敌方由 doSettlement 负责释放
    core::GameFSM fsm_;
    core::EnemySpawner spawner_;
    core::Shop shop_;
    std::vector<core::ItemType> pendingItems_;  // 已获得尚未装备的道具
    std::vector<core::DeploymentEntry> savedDeployment_;  // 上一次开战时的玩家部署
    int nextUnitId_;                            // 单位 ID 分配计数器（从 100 开始）

    // ── 战斗动画状态 ─────────────────────────────────────────────
    QTimer* battleTimer_;
    core::BattleEngine* battleEngine_;         // 当前战斗引擎，空闲时为 nullptr
    std::vector<core::Unit*> spawnedEnemies_;  // 本轮生成的敌方单位（用于结算时释放）
    std::vector<core::Unit*> playerUnits_;     // 玩家英雄指针（永久持有，用于每轮复活与内存管理）
    core::LevelConfig currentLevelCfg_;        // 本轮配置（奖励/惩罚值）
    static const int kTicksPerStep = 1;        // 每次定时器回调运行的 tick 数（1 = 最流畅动画）

    // ── Qt UI 组件 ───────────────────────────────────────────────
    ArenaScene* scene_;
    QGraphicsView* view_;
    UnitInfoPanel* infoPanel_;
    ShopPanel* shopPanel_;

    // 当前选中的单位 ID（-1 表示未选中）；装备道具时使用。
    int currentSelectedUnitId_;

    // 待装备道具面板（右侧栏）
    QWidget*  itemsWidget_;   // 容纳道具按钮的容器
    QVBoxLayout* itemsLayout_;  // itemsWidget_ 内部布局

    // 控制面板标签
    QLabel* phaseLabel_;
    QLabel* roundLabel_;
    QLabel* playerHpLabel_;
    QLabel* playerGoldLabel_;
    QLabel* populationLabel_;
    QLabel* synergyLabel_;

    // 控制按钮
    QPushButton* startBattleBtn_;
    QPushButton* deployFromLastBtn_;
    QPushButton* nextRoundBtn_;
    QPushButton* levelUpBtn_;
    QPushButton* saveBtn_;
    QPushButton* loadBtn_;

    // ── 辅助方法 ─────────────────────────────────────────────────
    void updateStatusPanel();
    void refreshPreparationSynergyBuffs();
    void updateSynergyDisplay();
    void updateShopDisplay();
    void updateItemsDisplay();  // 刷新右侧待装备道具面板
    void updateSelectedUnitPanel();
    void recomputeNextUnitId();
    void captureCurrentDeployment();
    int applySavedDeployment();
    bool hasSavedDeployment() const;
    QString settlementMessage(const core::RoundOutcome& outcome,
                              const std::vector<core::ItemType>& droppedItems) const;
    void showSettlementDialog(const core::RoundOutcome& outcome,
                              const std::vector<core::ItemType>& droppedItems);
    void doSettlement();  // 战斗结束后：结算金币/生命值、清理单位、切换 FSM

    bool hasEmptyBenchSlot() const;
    // 将新购英雄放置到备战区第一个空槽；返回是否成功。
    bool placeHeroOnBench(core::Unit* hero);
};

}  // namespace ui
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_UI_QT_QT_MAIN_WINDOW_H
