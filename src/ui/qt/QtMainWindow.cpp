#include "ui/qt/QtMainWindow.h"

#include <QGraphicsView>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>

#include "core/BattleEngine.h"
#include "ui/qt/ArenaScene.h"
#include "ui/qt/UnitInfoPanel.h"

namespace my_auto_arena {
namespace ui {

QtMainWindow::QtMainWindow(QWidget* parent)
    : QMainWindow(parent),
      board_(8, 8, 8),
      player_(1, 10, 100, 1, 3),
      fsm_(),
      spawner_(),
      nextUnitId_(100),
      battleTimer_(nullptr),
      battleEngine_(nullptr),
      scene_(nullptr),
      view_(nullptr),
      infoPanel_(nullptr),
      phaseLabel_(nullptr),
      roundLabel_(nullptr),
      playerHpLabel_(nullptr),
      playerGoldLabel_(nullptr),
      startBattleBtn_(nullptr),
      nextRoundBtn_(nullptr) {

    // ── 初始化玩家英雄单位 ────────────────────────────────────────
    // 使用五种英雄中的三种，便于 Phase 2 技能验收演示。
    core::Unit* hero1 = new core::AshRaiderHero(1, core::UnitOwner::player);
    core::Unit* hero2 = new core::NightArcherHero(2, core::UnitOwner::player);
    core::Unit* hero3 = new core::BonePrayerHero(3, core::UnitOwner::player);

    unitsMap_[hero1->id()] = hero1;
    unitsMap_[hero2->id()] = hero2;
    unitsMap_[hero3->id()] = hero3;

    player_.addUnit(1);
    player_.addUnit(2);
    player_.addUnit(3);

    board_.placeOnBench(1, 0);
    board_.placeOnBench(2, 1);
    board_.placeOnBench(3, 2);

    // ── 构建 UI 布局 ──────────────────────────────────────────────
    QWidget* central = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    setCentralWidget(central);

    // 上方：棋盘视图 + 信息面板
    QWidget* gameArea = new QWidget(central);
    QHBoxLayout* gameLayout = new QHBoxLayout(gameArea);

    scene_ = new ArenaScene(board_, player_, unitsMap_, this);
    view_ = new QGraphicsView(scene_, gameArea);
    view_->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    view_->setFixedSize(640, 660);

    infoPanel_ = new UnitInfoPanel(gameArea);
    infoPanel_->setFixedWidth(220);

    gameLayout->addWidget(view_);
    gameLayout->addWidget(infoPanel_);

    // 下方：控制面板
    QWidget* controlBar = new QWidget(central);
    controlBar->setStyleSheet("background-color: #1E1E2E; color: #CDD6F4; border-radius: 6px;");
    QHBoxLayout* controlLayout = new QHBoxLayout(controlBar);
    controlLayout->setContentsMargins(12, 8, 12, 8);
    controlLayout->setSpacing(16);

    phaseLabel_ = new QLabel("准备阶段", controlBar);
    phaseLabel_->setStyleSheet("font-weight: bold; font-size: 14px; color: #89DCEB; min-width: 80px;");

    roundLabel_ = new QLabel("第 1 轮", controlBar);
    roundLabel_->setStyleSheet("font-size: 13px; color: #A6E3A1; min-width: 60px;");

    playerHpLabel_ = new QLabel("HP: 100", controlBar);
    playerHpLabel_->setStyleSheet("font-size: 13px; color: #F38BA8; min-width: 70px;");

    playerGoldLabel_ = new QLabel("金币: 10", controlBar);
    playerGoldLabel_->setStyleSheet("font-size: 13px; color: #F9E2AF; min-width: 70px;");

    startBattleBtn_ = new QPushButton("⚔ 开始战斗", controlBar);
    startBattleBtn_->setFixedWidth(110);
    startBattleBtn_->setStyleSheet(
        "QPushButton { background-color: #F38BA8; color: #1E1E2E; font-weight: bold;"
        " border-radius: 5px; padding: 6px 12px; }"
        "QPushButton:disabled { background-color: #45475A; color: #6C7086; }");

    nextRoundBtn_ = new QPushButton("▶ 下一轮", controlBar);
    nextRoundBtn_->setFixedWidth(100);
    nextRoundBtn_->setEnabled(false);
    nextRoundBtn_->setStyleSheet(
        "QPushButton { background-color: #89DCEB; color: #1E1E2E; font-weight: bold;"
        " border-radius: 5px; padding: 6px 12px; }"
        "QPushButton:disabled { background-color: #45475A; color: #6C7086; }");

    controlLayout->addWidget(phaseLabel_);
    controlLayout->addWidget(roundLabel_);
    controlLayout->addWidget(playerHpLabel_);
    controlLayout->addWidget(playerGoldLabel_);
    controlLayout->addStretch();
    controlLayout->addWidget(startBattleBtn_);
    controlLayout->addWidget(nextRoundBtn_);

    mainLayout->addWidget(gameArea);
    mainLayout->addWidget(controlBar);

    connect(scene_, SIGNAL(unitSelected(int)), this, SLOT(onUnitSelected(int)));
    connect(scene_, SIGNAL(dragResultReady(core::DragResult)), this, SLOT(onDragResult(core::DragResult)));
    connect(startBattleBtn_, SIGNAL(clicked()), this, SLOT(onStartBattle()));
    connect(nextRoundBtn_, SIGNAL(clicked()), this, SLOT(onNextRound()));

    battleTimer_ = new QTimer(this);
    battleTimer_->setSingleShot(false);
    connect(battleTimer_, SIGNAL(timeout()), this, SLOT(onBattleTick()));

    setWindowTitle("Synera: Synergy Auto-Arena — Phase 1 + Phase 2");
    resize(880, 780);
    statusBar()->showMessage("将英雄拖入下半场，点击「开始战斗」");
}

QtMainWindow::~QtMainWindow() {
    // 确保定时器已停止，避免析构期间回调。
    if (battleTimer_ != nullptr) {
        battleTimer_->stop();
    }
    delete battleEngine_;
    battleEngine_ = nullptr;

    // 释放所有存活单位（玩家单位 + 可能残留的敌方单位）。
    for (std::map<int, core::Unit*>::iterator it = unitsMap_.begin(); it != unitsMap_.end(); ++it) {
        delete it->second;
    }
    // 释放已从 unitsMap_ 移除但尚未 delete 的战斗阵亡敌方单位。
    for (std::size_t i = 0; i < spawnedEnemies_.size(); ++i) {
        core::Unit* unit = spawnedEnemies_.at(i);
        if (unitsMap_.find(unit->id()) == unitsMap_.end()) {
            delete unit;
        }
    }
}

void QtMainWindow::onUnitSelected(int unitId) { infoPanel_->setUnit(scene_->unitById(unitId)); }

void QtMainWindow::onDragResult(core::DragResult result) {
    if (result == core::DragResult::kSuccess) {
        statusBar()->showMessage("放置成功", 1500);
    } else if (result == core::DragResult::kSwapped) {
        statusBar()->showMessage("交换成功", 1500);
    } else if (result == core::DragResult::kNotPlayerHalf) {
        statusBar()->showMessage("非法位置：非玩家半场", 1800);
    } else if (result == core::DragResult::kPopulationFull) {
        statusBar()->showMessage("人口已满", 1800);
    } else if (result == core::DragResult::kOutOfBounds) {
        statusBar()->showMessage("越界：已回弹", 1800);
    } else if (result == core::DragResult::kSameLocation) {
        statusBar()->showMessage("原地放下", 1200);
    } else {
        statusBar()->showMessage("非法操作", 1500);
    }
}

void QtMainWindow::onStartBattle() {
    if (!fsm_.canPlayerAct()) {
        statusBar()->showMessage("当前阶段不可操作", 1500);
        return;
    }
    if (fsm_.currentRound() > 6) {
        QMessageBox::information(this, "恭喜通关", "已通过全部 6 关！");
        return;
    }

    // 阶段切换：准备 → 战斗
    fsm_.startBattle();
    scene_->setDragEnabled(false);
    startBattleBtn_->setEnabled(false);
    nextRoundBtn_->setEnabled(false);
    updateStatusPanel();

    const int round = fsm_.currentRound();
    currentLevelCfg_ = spawner_.configForRound(round);
    statusBar()->showMessage(QString("第 %1 轮：战斗开始！").arg(round));

    // 生成敌方单位并添加到场景
    spawnedEnemies_ = spawner_.spawnRound(round, board_, nextUnitId_);
    for (std::size_t i = 0; i < spawnedEnemies_.size(); ++i) {
        core::Unit* unit = spawnedEnemies_.at(i);
        unitsMap_[unit->id()] = unit;
        scene_->addUnitItem(unit);
    }

    // 创建战斗引擎并启动定时器（每 150ms 推进若干 tick）
    battleEngine_ = new core::BattleEngine(board_, unitsMap_);
    battleEngine_->setDefeatHpPenalty(currentLevelCfg_.onLosePlayerHpDamage);
    battleTimer_->start(150);
}

void QtMainWindow::onBattleTick() {
    if (battleEngine_ == nullptr) {
        battleTimer_->stop();
        return;
    }

    // 每次定时器触发推进若干 tick，让移动和技能触发有时间显示在画面上
    for (int i = 0; i < kTicksPerStep && !battleEngine_->isFinished(); ++i) {
        battleEngine_->tick();
    }

    // 刷新单位位置与血蓝条；移除阵亡单位图元
    scene_->syncAfterBattle(unitsMap_);
    statusBar()->showMessage(QString("⚔ 战斗中... Tick %1").arg(battleEngine_->tickCount()));

    if (battleEngine_->isFinished()) {
        battleTimer_->stop();
        doSettlement();
    }
}

void QtMainWindow::doSettlement() {
    if (battleEngine_ == nullptr) {
        return;
    }

    core::RoundOutcome outcome = battleEngine_->outcome();

    // 释放战斗期间阵亡的敌方单位（已被 BattleEngine 从 unitsMap_ 移除但未 delete）
    for (std::size_t i = 0; i < spawnedEnemies_.size(); ++i) {
        core::Unit* unit = spawnedEnemies_.at(i);
        if (unitsMap_.find(unit->id()) == unitsMap_.end()) {
            delete unit;
        }
    }
    spawnedEnemies_.clear();

    // 结算金币 / 玩家 HP
    if (outcome.playerWon) {
        outcome.goldReward = currentLevelCfg_.winGoldReward;
        player_.setGold(player_.gold() + outcome.goldReward);
        outcome.gameOver = false;
    } else {
        outcome.hpPenalty = currentLevelCfg_.onLosePlayerHpDamage;
        const int newHp = player_.hp() - outcome.hpPenalty;
        player_.setHp(newHp < 0 ? 0 : newHp);
        outcome.gameOver = (player_.hp() <= 0);
    }

    // 移除存活的敌方单位（清理 Board 与 unitsMap_，delete 指针）
    core::PvERoundRunner::removeEnemyUnits(board_, unitsMap_);
    scene_->syncAfterBattle(unitsMap_);

    delete battleEngine_;
    battleEngine_ = nullptr;

    // FSM：战斗 → 结算
    fsm_.startSettlement(outcome);
    updateStatusPanel();

    // 显示本轮结果
    if (outcome.playerWon) {
        statusBar()->showMessage(
            QString("胜利！获得 %1 金币 | HP: %2").arg(outcome.goldReward).arg(player_.hp()));
    } else {
        statusBar()->showMessage(
            QString("失败！失去 %1 HP | 剩余 HP: %2").arg(outcome.hpPenalty).arg(player_.hp()));
    }

    if (outcome.gameOver) {
        QMessageBox::critical(this, "游戏结束", "玩家血量耗尽，游戏结束！");
        startBattleBtn_->setEnabled(false);
        nextRoundBtn_->setEnabled(false);
        return;
    }

    nextRoundBtn_->setEnabled(true);
}

void QtMainWindow::onNextRound() {
    if (fsm_.currentPhase() != core::GamePhase::kSettlement) {
        return;
    }
    if (fsm_.currentRound() >= 6) {
        QMessageBox::information(this, "恭喜通关", "已通过全部 6 关！");
        nextRoundBtn_->setEnabled(false);
        return;
    }

    // 结算 → 准备
    fsm_.startNextRound();
    scene_->setDragEnabled(true);
    startBattleBtn_->setEnabled(true);
    nextRoundBtn_->setEnabled(false);
    updateStatusPanel();
    statusBar()->showMessage(QString("第 %1 轮准备中，请布置英雄阵型").arg(fsm_.currentRound()));
}

void QtMainWindow::updateStatusPanel() {
    const QString phaseText =
        (fsm_.currentPhase() == core::GamePhase::kPrepare)    ? "准备阶段"
        : (fsm_.currentPhase() == core::GamePhase::kBattle)   ? "战斗阶段"
                                                              : "结算阶段";
    phaseLabel_->setText(phaseText);
    roundLabel_->setText(QString("第 %1 轮").arg(fsm_.currentRound()));
    playerHpLabel_->setText(QString("HP: %1").arg(player_.hp()));
    playerGoldLabel_->setText(QString("金币: %1").arg(player_.gold()));
}

}  // namespace ui
}  // namespace my_auto_arena
