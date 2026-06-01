#include "ui/qt/QtMainWindow.h"

#include <QFileDialog>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>

#include "core/BattleEngine.h"
#include "ui/qt/ArenaScene.h"
#include "ui/qt/ShopPanel.h"
#include "ui/qt/UnitInfoPanel.h"

namespace my_auto_arena {
namespace ui {

// 流程：创建开局英雄并放备战区 ──> 搭建场景/面板/控制栏 ──> 连接信号 ──> 初始化显示
QtMainWindow::QtMainWindow(QWidget* parent)
    : QMainWindow(parent),
      board_(8, 8, 8),
      player_(1, 8, 100, 1, 2),
      fsm_(),
      spawner_(),
      shop_(),
      nextUnitId_(100),
      currentSelectedUnitId_(-1),
      battleTimer_(nullptr),
      battleEngine_(nullptr),
      scene_(nullptr),
      view_(nullptr),
      infoPanel_(nullptr),
      shopPanel_(nullptr),
      itemsWidget_(nullptr),
      itemsLayout_(nullptr),
      phaseLabel_(nullptr),
      roundLabel_(nullptr),
      playerHpLabel_(nullptr),
      playerGoldLabel_(nullptr),
      populationLabel_(nullptr),
      synergyLabel_(nullptr),
      startBattleBtn_(nullptr),
      nextRoundBtn_(nullptr),
      levelUpBtn_(nullptr),
      saveBtn_(nullptr),
      loadBtn_(nullptr) {

    // ── 初始化玩家英雄单位 ──────────────────────────────────────────
    // 开局仅给2个英雄（战士+射手），迫使玩家用有限金币做第一次英雄选择。
    core::Unit* hero1 = new core::AshRaiderHero(1, core::UnitOwner::player);
    core::Unit* hero2 = new core::NightArcherHero(2, core::UnitOwner::player);

    unitsMap_[hero1->id()] = hero1;
    unitsMap_[hero2->id()] = hero2;

    playerUnits_.push_back(hero1);
    playerUnits_.push_back(hero2);

    player_.addUnit(1);
    player_.addUnit(2);

    board_.placeOnBench(1, 0);
    board_.placeOnBench(2, 1);

    // ── 构建 UI 布局 ──────────────────────────────────────────────
    QWidget* central = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(6);
    setCentralWidget(central);

    // 上方：棋盘视图 + 信息面板
    QWidget* gameArea = new QWidget(central);
    QHBoxLayout* gameLayout = new QHBoxLayout(gameArea);

    scene_ = new ArenaScene(board_, player_, unitsMap_, this);
    view_ = new QGraphicsView(scene_, gameArea);
    view_->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    view_->setFixedSize(640, 660);

    // 右侧信息+商店面板。内容可能随装备/羁绊增长，放入滚动区避免挤压遮挡。
    QScrollArea* rightScroll = new QScrollArea(gameArea);
    rightScroll->setWidgetResizable(true);
    rightScroll->setFixedWidth(260);
    rightScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    rightScroll->setStyleSheet("QScrollArea { border: none; background: transparent; }");

    QWidget* rightPanel = new QWidget(rightScroll);
    rightPanel->setFixedWidth(240);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(6);

    infoPanel_ = new UnitInfoPanel(rightPanel);
    shopPanel_ = new ShopPanel(rightPanel);

    synergyLabel_ = new QLabel("羁绊: -", rightPanel);
    synergyLabel_->setStyleSheet(
        "background-color: #181825; color: #CBA6F7; font-size: 11px;"
        " border-radius: 4px; padding: 4px;");
    synergyLabel_->setWordWrap(true);

    // 待装备道具面板
    QLabel* itemsTitle = new QLabel("⚗ 待装备道具", rightPanel);
    itemsTitle->setStyleSheet("color: #FAB387; font-weight: bold; font-size: 12px; padding-top: 4px;");

    itemsWidget_ = new QWidget(rightPanel);
    itemsWidget_->setStyleSheet("background-color: #181825; border-radius: 4px;");
    itemsLayout_ = new QVBoxLayout(itemsWidget_);
    itemsLayout_->setContentsMargins(4, 4, 4, 4);
    itemsLayout_->setSpacing(3);

    rightLayout->addWidget(infoPanel_);
    rightLayout->addWidget(synergyLabel_);
    rightLayout->addWidget(shopPanel_);
    rightLayout->addWidget(itemsTitle);
    rightLayout->addWidget(itemsWidget_);
    rightLayout->addStretch();
    rightScroll->setWidget(rightPanel);

    gameLayout->addWidget(view_);
    gameLayout->addWidget(rightScroll);

    // 下方：控制面板
    QWidget* controlBar = new QWidget(central);
    controlBar->setStyleSheet("background-color: #1E1E2E; color: #CDD6F4; border-radius: 6px;");
    QHBoxLayout* controlLayout = new QHBoxLayout(controlBar);
    controlLayout->setContentsMargins(12, 8, 12, 8);
    controlLayout->setSpacing(10);

    phaseLabel_ = new QLabel("准备阶段", controlBar);
    phaseLabel_->setStyleSheet("font-weight: bold; font-size: 14px; color: #89DCEB; min-width: 80px;");

    roundLabel_ = new QLabel("第 1 轮", controlBar);
    roundLabel_->setStyleSheet("font-size: 13px; color: #A6E3A1; min-width: 60px;");

    playerHpLabel_ = new QLabel("HP: 100", controlBar);
    playerHpLabel_->setStyleSheet("font-size: 13px; color: #F38BA8; min-width: 70px;");

    playerGoldLabel_ = new QLabel("金币: 10", controlBar);
    playerGoldLabel_->setStyleSheet("font-size: 13px; color: #F9E2AF; min-width: 70px;");

    populationLabel_ = new QLabel("人口: 0/2", controlBar);
    populationLabel_->setStyleSheet("font-size: 13px; color: #CDD6F4; min-width: 70px;");

    const QString btnStyle =
        "QPushButton { background-color: %1; color: #1E1E2E; font-weight: bold;"
        " border-radius: 5px; padding: 5px 10px; }"
        "QPushButton:disabled { background-color: #45475A; color: #6C7086; }";

    startBattleBtn_ = new QPushButton("⚔ 开始战斗", controlBar);
    startBattleBtn_->setStyleSheet(btnStyle.arg("#F38BA8"));

    nextRoundBtn_ = new QPushButton("▶ 下一轮", controlBar);
    nextRoundBtn_->setEnabled(false);
    nextRoundBtn_->setStyleSheet(btnStyle.arg("#89DCEB"));

    levelUpBtn_ = new QPushButton("升级人口", controlBar);
    levelUpBtn_->setStyleSheet(btnStyle.arg("#A6E3A1"));

    saveBtn_ = new QPushButton("存档", controlBar);
    saveBtn_->setStyleSheet(btnStyle.arg("#CBA6F7"));

    loadBtn_ = new QPushButton("读档", controlBar);
    loadBtn_->setStyleSheet(btnStyle.arg("#FAB387"));

    controlLayout->addWidget(phaseLabel_);
    controlLayout->addWidget(roundLabel_);
    controlLayout->addWidget(playerHpLabel_);
    controlLayout->addWidget(playerGoldLabel_);
    controlLayout->addWidget(populationLabel_);
    controlLayout->addStretch();
    controlLayout->addWidget(levelUpBtn_);
    controlLayout->addWidget(saveBtn_);
    controlLayout->addWidget(loadBtn_);
    controlLayout->addWidget(startBattleBtn_);
    controlLayout->addWidget(nextRoundBtn_);

    mainLayout->addWidget(gameArea);
    mainLayout->addWidget(controlBar);

    // ── 信号连接 ─────────────────────────────────────────────────
    connect(scene_, SIGNAL(unitSelected(int)), this, SLOT(onUnitSelected(int)));
    connect(scene_, SIGNAL(dragResultReady(core::DragResult)), this, SLOT(onDragResult(core::DragResult)));
    connect(startBattleBtn_, SIGNAL(clicked()), this, SLOT(onStartBattle()));
    connect(nextRoundBtn_, SIGNAL(clicked()), this, SLOT(onNextRound()));
    connect(shopPanel_, SIGNAL(heroPurchased(int)), this, SLOT(onHeroPurchased(int)));
    connect(shopPanel_, SIGNAL(refreshRequested()), this, SLOT(onShopRefresh()));
    connect(infoPanel_, SIGNAL(sellRequested(int)), this, SLOT(onSellUnit(int)));
    connect(infoPanel_, SIGNAL(unequipRequested(int, int)), this, SLOT(onUnequipItem(int, int)));
    connect(levelUpBtn_, SIGNAL(clicked()), this, SLOT(onLevelUp()));
    connect(saveBtn_, SIGNAL(clicked()), this, SLOT(onSaveGame()));
    connect(loadBtn_, SIGNAL(clicked()), this, SLOT(onLoadGame()));

    battleTimer_ = new QTimer(this);
    battleTimer_->setSingleShot(false);
    battleTimer_->setInterval(250);
    connect(battleTimer_, SIGNAL(timeout()), this, SLOT(onBattleTick()));

    refreshPreparationSynergyBuffs();
    updateStatusPanel();
    updateShopDisplay();
    updateSynergyDisplay();
    updateItemsDisplay();

    setWindowTitle("Synera: Synergy Auto-Arena — Phase 3");
    resize(920, 800);
    statusBar()->showMessage("将英雄拖入下半场，点击「开始战斗」");
}

// 流程：停战斗定时器 ──> 释放战斗引擎 ──> delete 所有单位指针
QtMainWindow::~QtMainWindow() {
    if (battleTimer_ != nullptr) {
        battleTimer_->stop();
    }
    delete battleEngine_;
    battleEngine_ = nullptr;

    for (std::map<int, core::Unit*>::iterator it = unitsMap_.begin(); it != unitsMap_.end(); ++it) {
        delete it->second;
    }
    for (std::size_t i = 0; i < spawnedEnemies_.size(); ++i) {
        core::Unit* unit = spawnedEnemies_.at(i);
        if (unitsMap_.find(unit->id()) == unitsMap_.end()) {
            delete unit;
        }
    }
    for (std::size_t i = 0; i < playerUnits_.size(); ++i) {
        core::Unit* unit = playerUnits_.at(i);
        if (unit != nullptr && unitsMap_.find(unit->id()) == unitsMap_.end()) {
            delete unit;
        }
    }
}

void QtMainWindow::onUnitSelected(int unitId) {
    currentSelectedUnitId_ = unitId;
    infoPanel_->setUnit(scene_->unitById(unitId));
    updateItemsDisplay();  // 选中单位后刷新道具按钮的可用状态
}

// 流程：按 DragResult 类型 ──> 更新状态栏提示 ──> 刷新状态与羁绊显示
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
    refreshPreparationSynergyBuffs();
    updateStatusPanel();
}

// 流程：校验可操作与棋盘人口 ──> 切 FSM 到战斗 ──> 生成敌方 ──> 施加羁绊 ──> 启动 BattleEngine 与定时器
void QtMainWindow::onStartBattle() {
    if (!fsm_.canPlayerAct()) {
        statusBar()->showMessage("当前阶段不可操作", 1500);
        return;
    }
    if (player_.populationOnBoard(board_) == 0) {
        statusBar()->showMessage("请先将英雄拖入下半场棋盘再开始战斗！", 2500);
        return;
    }

    fsm_.startBattle();
    scene_->setDragEnabled(false);
    startBattleBtn_->setEnabled(false);
    nextRoundBtn_->setEnabled(false);
    updateStatusPanel();

    const int round = fsm_.currentRound();
    currentLevelCfg_ = spawner_.configForRound(round);
    statusBar()->showMessage(QString("第 %1 轮：战斗开始！").arg(round));

    // 生成敌方单位并添加到场景。
    spawnedEnemies_ = spawner_.spawnRound(round, board_, nextUnitId_);
    for (std::size_t i = 0; i < spawnedEnemies_.size(); ++i) {
        core::Unit* unit = spawnedEnemies_.at(i);
        unitsMap_[unit->id()] = unit;
        scene_->addUnitItem(unit);
    }

    // 战斗开始前重新施加羁绊 BUFF，确保与备战阶段展示的数值一致。
    core::SynergySystem::applyBuffs(board_, unitsMap_);
    updateSynergyDisplay();
    updateSelectedUnitPanel();

    battleEngine_ = new core::BattleEngine(board_, unitsMap_);
    battleEngine_->setDefeatHpPenalty(currentLevelCfg_.onLosePlayerHpDamage);
    battleTimer_->start();
}

// 流程：推进若干 tick ──> 同步场景与特效 ──> 战斗结束则 doSettlement
void QtMainWindow::onBattleTick() {
    if (battleEngine_ == nullptr) {
        battleTimer_->stop();
        return;
    }
    for (int i = 0; i < kTicksPerStep && !battleEngine_->isFinished(); ++i) {
        battleEngine_->tick();
    }
    // syncAfterBattle() 先清除上一 tick 特效并更新单位位置，之后再叠加本 tick 新特效。
    scene_->syncAfterBattle(unitsMap_);
    scene_->spawnVfx(battleEngine_->lastTickEvents());
    statusBar()->showMessage(QString("⚔ 战斗中... Tick %1").arg(battleEngine_->tickCount()));

    if (battleEngine_->isFinished()) {
        battleTimer_->stop();
        doSettlement();
    }
}

// 流程：取战斗结果 ──> 结算金币/扣血/掉落 ──> 清理敌方 ──> 清除羁绊 ──> 复活玩家英雄回备战区
//       ──> 尝试升星 ──> 切 FSM 到结算 ──> 判断是否 gameOver
void QtMainWindow::doSettlement() {
    if (battleEngine_ == nullptr) {
        return;
    }

    core::RoundOutcome outcome = battleEngine_->outcome();

    // 释放战斗期间阵亡的敌方单位。
    for (std::size_t i = 0; i < spawnedEnemies_.size(); ++i) {
        core::Unit* unit = spawnedEnemies_.at(i);
        if (unitsMap_.find(unit->id()) == unitsMap_.end()) {
            delete unit;
        }
    }
    spawnedEnemies_.clear();

    // 结算金币 / 玩家生命值。
    if (outcome.playerWon) {
        outcome.goldReward = currentLevelCfg_.winGoldReward;
        player_.setGold(player_.gold() + outcome.goldReward);
        outcome.gameOver = false;

        // 胜利时随机给予一件道具（轮次 >= 2 才开始掉落）。
        if (fsm_.currentRound() >= 2) {
            const core::ItemType drops[7] = {
                core::ItemType::kSword,        core::ItemType::kArmor,
                core::ItemType::kRing,         core::ItemType::kTalisman,
                core::ItemType::kRunicShield,  core::ItemType::kSwiftGloves,
                core::ItemType::kBlueCrystal
            };
            pendingItems_.push_back(drops[std::rand() % 7]);
        }
    } else {
        outcome.hpPenalty = currentLevelCfg_.onLosePlayerHpDamage;
        const int newHp = player_.hp() - outcome.hpPenalty;
        player_.setHp(newHp < 0 ? 0 : newHp);
        outcome.gameOver = (player_.hp() <= 0);
    }

    // 移除存活的敌方单位（清理 Board 与 unitsMap_）。
    core::PvERoundRunner::removeEnemyUnits(board_, unitsMap_);
    scene_->syncAfterBattle(unitsMap_);

    // 清除羁绊 BUFF（战斗结束后所有玩家单位清零羁绊加成）。
    core::SynergySystem::clearBuffs(playerUnits_);

    // 复活并归还所有玩家英雄。
    for (std::size_t i = 0; i < playerUnits_.size(); ++i) {
        core::Unit* hero = playerUnits_.at(i);
        unitsMap_[hero->id()] = hero;
        hero->resetToFull();
    }
    for (std::size_t i = 0; i < playerUnits_.size(); ++i) {
        const int heroId = playerUnits_.at(i)->id();
        const core::Position pos = board_.findUnitOnBoard(heroId);
        if (board_.inBounds(pos)) {
            board_.clearOnBoard(pos);
        }
        for (int slot = 0; slot < board_.benchSize(); ++slot) {
            if (board_.occupantOnBench(slot) == heroId) {
                board_.clearOnBench(slot);
                break;
            }
        }
    }
    for (std::size_t i = 0; i < playerUnits_.size(); ++i) {
        const int heroId = playerUnits_.at(i)->id();
        for (int slot = 0; slot < board_.benchSize(); ++slot) {
            if (board_.occupantOnBench(slot) == core::Board::kEmptySlot) {
                board_.placeOnBench(heroId, slot);
                break;
            }
        }
    }

    scene_->syncAfterBattle(unitsMap_);
    for (std::size_t i = 0; i < playerUnits_.size(); ++i) {
        scene_->addUnitItem(playerUnits_.at(i));
    }
    scene_->rebuild();
    scene_->setDragEnabled(true);

    delete battleEngine_;
    battleEngine_ = nullptr;

    // 检查升星（复活后）。
    core::StarUpgrade::tryMergeAll(playerUnits_, board_, unitsMap_, player_, &pendingItems_);

    updateShopDisplay();
    updateSynergyDisplay();

    fsm_.startSettlement(outcome);
    updateStatusPanel();

    updateItemsDisplay();

    if (!pendingItems_.empty()) {
        statusBar()->showMessage(
            QString("获得道具: %1 | 点击英雄再点道具按钮装备")
                .arg(QString::fromStdString(core::getItemDef(pendingItems_.back()).name)));
    } else if (outcome.playerWon) {
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

// 流程：校验阶段 ──> FSM 进入下一轮 ──> 刷新商店 ──> 恢复拖拽与按钮状态
void QtMainWindow::onNextRound() {
    if (fsm_.currentPhase() != core::GamePhase::kSettlement) {
        return;
    }
    fsm_.startNextRound();
    // 每轮开始时刷新商店（新一轮新货架）。
    shop_ = core::Shop();
    scene_->setDragEnabled(true);
    startBattleBtn_->setEnabled(true);
    nextRoundBtn_->setEnabled(false);
    refreshPreparationSynergyBuffs();
    updateStatusPanel();
    updateShopDisplay();
    statusBar()->showMessage(QString("第 %1 轮准备中，请布置英雄阵型").arg(fsm_.currentRound()));
}

// 流程：校验阶段与备战区空位 ──> 商店购买 ──> 放备战区 ──> 注册单位 ──> 尝试升星合并
void QtMainWindow::onHeroPurchased(int slotIndex) {
    if (!fsm_.canPlayerAct()) {
        statusBar()->showMessage("战斗阶段无法购买英雄", 1500);
        return;
    }
    if (!hasEmptyBenchSlot()) {
        statusBar()->showMessage("备战区已满，无法购买", 1800);
        return;
    }

    int gold = player_.gold();
    core::Unit* hero = shop_.buy(slotIndex, gold, nextUnitId_++);
    if (hero == nullptr) {
        statusBar()->showMessage("金币不足或槽位已售出", 1500);
        return;
    }
    player_.setGold(gold);

    if (!placeHeroOnBench(hero)) {
        shop_.cancelSlotSale(slotIndex);
        player_.setGold(player_.gold() + core::Shop::kHeroCost);
        delete hero;
        statusBar()->showMessage("备战区已满，无法购买", 1800);
        updateShopDisplay();
        return;
    }

    // 在 tryMergeAll 之前保存名称，防止 hero 被合并后指针悬空导致崩溃。
    const std::string heroName = hero->name();
    unitsMap_[hero->id()] = hero;
    playerUnits_.push_back(hero);
    player_.addUnit(hero->id());
    scene_->addUnitItem(hero);
    scene_->rebuild();

    // 检查升星（可能删除 hero 指针，此后不得再访问 hero）。
    const bool merged = core::StarUpgrade::tryMergeAll(
        playerUnits_, board_, unitsMap_, player_, &pendingItems_);
    scene_->syncAfterBattle(unitsMap_);
    scene_->rebuild();

    refreshPreparationSynergyBuffs();
    updateStatusPanel();
    updateShopDisplay();
    updateItemsDisplay();
    statusBar()->showMessage(
        merged ? QString("购买 %1 成功，触发升星合并！").arg(QString::fromStdString(heroName))
               : QString("购买 %1 成功").arg(QString::fromStdString(heroName)),
        2000);
}

void QtMainWindow::onShopRefresh() {
    if (!fsm_.canPlayerAct()) {
        statusBar()->showMessage("战斗阶段无法刷新商店", 1500);
        return;
    }
    int gold = player_.gold();
    shop_.refresh(gold);
    player_.setGold(gold);
    updateStatusPanel();
    updateShopDisplay();
}

// 流程：查找玩家单位 ──> 加金币 ──> 清棋盘/备战区占位 ──> 从列表与 map 移除 ──> delete
void QtMainWindow::onSellUnit(int unitId) {
    if (!fsm_.canPlayerAct()) {
        statusBar()->showMessage("战斗阶段无法出售英雄", 1500);
        return;
    }

    // 查找单位。
    std::map<int, core::Unit*>::iterator it = unitsMap_.find(unitId);
    if (it == unitsMap_.end() || it->second == nullptr) {
        return;
    }
    core::Unit* unit = it->second;
    if (unit->owner() != core::UnitOwner::player) {
        return;
    }

    const int gain = core::Shop::sellValue(unit->starLevel());
    player_.setGold(player_.gold() + gain);

    // 从棋盘/备战区移除占位。
    const core::Position pos = board_.findUnitOnBoard(unitId);
    if (board_.inBounds(pos)) {
        board_.clearOnBoard(pos);
    }
    for (int slot = 0; slot < board_.benchSize(); ++slot) {
        if (board_.occupantOnBench(slot) == unitId) {
            board_.clearOnBench(slot);
            break;
        }
    }

    // 从场景移除图元。
    scene_->syncAfterBattle(unitsMap_);

    // 从玩家单位列表中移除。
    for (std::size_t i = 0; i < playerUnits_.size(); ++i) {
        if (playerUnits_.at(i) != nullptr && playerUnits_.at(i)->id() == unitId) {
            playerUnits_.erase(playerUnits_.begin() + static_cast<int>(i));
            break;
        }
    }
    player_.removeUnit(unitId);
    unitsMap_.erase(unitId);

    // 从 unitsMap_ 移除后再同步场景（场景会删除对应图元）。
    scene_->syncAfterBattle(unitsMap_);
    scene_->rebuild();
    infoPanel_->setUnit(nullptr);
    currentSelectedUnitId_ = -1;

    const std::vector<core::ItemType> returnedItems = unit->takeAllEquippedItems();
    for (std::size_t i = 0; i < returnedItems.size(); ++i) {
        pendingItems_.push_back(returnedItems.at(i));
    }
    delete unit;

    refreshPreparationSynergyBuffs();
    updateStatusPanel();
    updateShopDisplay();
    updateItemsDisplay();
    statusBar()->showMessage(QString("出售英雄，获得 %1 金币").arg(gain), 1500);
}

// 流程：校验阶段与金币 ──> 扣费并 populationCap+1 ──> 刷新状态栏
void QtMainWindow::onLevelUp() {
    if (!fsm_.canPlayerAct()) {
        statusBar()->showMessage("战斗阶段无法升级", 1500);
        return;
    }
    const int currentCap = player_.populationCap();
    if (currentCap >= 8) {
        statusBar()->showMessage("人口上限已达最大值 8", 1500);
        return;
    }
    const int cost = currentCap * 2;
    if (player_.gold() < cost) {
        statusBar()->showMessage(QString("金币不足，升级需要 %1 金").arg(cost), 1800);
        return;
    }
    player_.setGold(player_.gold() - cost);
    player_.setPopulationCap(currentCap + 1);
    updateStatusPanel();
    statusBar()->showMessage(QString("人口上限提升到 %1").arg(player_.populationCap()), 1500);
}

void QtMainWindow::onSaveGame() {
    if (fsm_.currentPhase() == core::GamePhase::kBattle) {
        statusBar()->showMessage("战斗阶段暂不能存档，请等待结算后再保存", 2000);
        return;
    }
    const QString filepath = QFileDialog::getSaveFileName(
        this, "保存游戏", "save_game.txt", "文本文件 (*.txt)");
    if (filepath.isEmpty()) return;

    const bool ok = core::SaveManager::save(
        filepath.toStdString(), fsm_, player_, board_, playerUnits_, pendingItems_);
    if (ok) {
        statusBar()->showMessage("存档成功: " + filepath, 2000);
    } else {
        QMessageBox::warning(this, "存档失败", "无法写入存档文件！");
    }
}

// 流程：选档路径 ──> SaveManager::load ──> 重建场景与各项 UI 显示
void QtMainWindow::onLoadGame() {
    if (fsm_.currentPhase() == core::GamePhase::kBattle) {
        statusBar()->showMessage("战斗阶段暂不能读档，请等待结算后再读取", 2000);
        return;
    }
    const QString filepath = QFileDialog::getOpenFileName(
        this, "读取存档", "", "文本文件 (*.txt)");
    if (filepath.isEmpty()) return;

    const bool ok = core::SaveManager::load(
        filepath.toStdString(), fsm_, player_, board_, playerUnits_, unitsMap_, pendingItems_);
    if (ok) {
        recomputeNextUnitId();
        scene_->syncAfterBattle(unitsMap_);
        for (std::size_t i = 0; i < playerUnits_.size(); ++i) {
            scene_->addUnitItem(playerUnits_.at(i));
        }
        scene_->rebuild();
        refreshPreparationSynergyBuffs();
        updateStatusPanel();
        updateShopDisplay();
        updateItemsDisplay();
        statusBar()->showMessage("读档成功", 2000);
    } else {
        QMessageBox::warning(this, "读档失败", "无法读取存档文件或格式错误！");
    }
}

bool QtMainWindow::hasEmptyBenchSlot() const {
    for (int slot = 0; slot < board_.benchSize(); ++slot) {
        if (board_.occupantOnBench(slot) == core::Board::kEmptySlot) {
            return true;
        }
    }
    return false;
}

bool QtMainWindow::placeHeroOnBench(core::Unit* hero) {
    for (int slot = 0; slot < board_.benchSize(); ++slot) {
        if (board_.occupantOnBench(slot) == core::Board::kEmptySlot) {
            board_.placeOnBench(hero->id(), slot);
            return true;
        }
    }
    return false;
}

void QtMainWindow::refreshPreparationSynergyBuffs() {
    if (fsm_.canPlayerAct()) {
        core::SynergySystem::applyBuffs(board_, unitsMap_);
    }
    updateSynergyDisplay();
    updateSelectedUnitPanel();
}

void QtMainWindow::updateSelectedUnitPanel() {
    if (infoPanel_ == nullptr) {
        return;
    }
    if (currentSelectedUnitId_ < 0) {
        return;
    }
    std::map<int, core::Unit*>::iterator it = unitsMap_.find(currentSelectedUnitId_);
    if (it == unitsMap_.end() || it->second == nullptr) {
        infoPanel_->setUnit(nullptr);
        currentSelectedUnitId_ = -1;
        return;
    }
    infoPanel_->setUnit(it->second);
}

void QtMainWindow::recomputeNextUnitId() {
    int maxId = 99;
    for (std::map<int, core::Unit*>::const_iterator it = unitsMap_.begin(); it != unitsMap_.end(); ++it) {
        if (it->first > maxId) {
            maxId = it->first;
        }
    }
    nextUnitId_ = maxId + 1;
}

// 流程：刷新阶段/轮次/HP/金币/人口标签 ──> 更新升级人口按钮文案与可用性
void QtMainWindow::updateStatusPanel() {
    const QString phaseText =
        (fsm_.currentPhase() == core::GamePhase::kPrepare)  ? "准备阶段"
        : (fsm_.currentPhase() == core::GamePhase::kBattle) ? "战斗阶段"
                                                             : "结算阶段";
    phaseLabel_->setText(phaseText);
    roundLabel_->setText(QString("第 %1 轮").arg(fsm_.currentRound()));
    playerHpLabel_->setText(QString("HP: %1").arg(player_.hp()));
    playerGoldLabel_->setText(QString("金币: %1").arg(player_.gold()));

    const int onBoard = player_.populationOnBoard(board_);
    populationLabel_->setText(
        QString("人口: %1/%2").arg(onBoard).arg(player_.populationCap()));

    // 升级按钮显示当前费用。
    if (player_.populationCap() < 8) {
        const int cost = player_.populationCap() * 2;
        levelUpBtn_->setText(QString("升级人口 -%1金").arg(cost));
        levelUpBtn_->setEnabled(fsm_.canPlayerAct() && player_.gold() >= cost);
    } else {
        levelUpBtn_->setText("人口已满");
        levelUpBtn_->setEnabled(false);
    }
}

// 流程：查询全部羁绊 ──> 拼接星数/下一等级/当前增益 ──> 设置悬停详情
void QtMainWindow::updateSynergyDisplay() {
    if (synergyLabel_ == nullptr) return;
    const std::vector<core::ActiveSynergy> synergies =
        core::SynergySystem::getActiveSynergies(board_, unitsMap_);

    QString text = "羁绊\n";
    QString tooltip;
    for (std::size_t i = 0; i < synergies.size(); ++i) {
        const core::ActiveSynergy& s = synergies.at(i);
        const int displayThreshold = (s.activeThreshold > 0 && s.count >= s.nextThreshold)
                                         ? s.activeThreshold
                                         : s.nextThreshold;
        text += QString("%1  %2/%3\n%4\n")
                    .arg(QString::fromStdString(s.name))
                    .arg(s.count)
                    .arg(displayThreshold)
                    .arg(QString::fromStdString(s.buffDescription));
        if (i + 1 < synergies.size()) {
            text += "\n";
        }
        tooltip += QString("%1\n职业: %2\n阶段:\n%3\n")
                       .arg(QString::fromStdString(s.name))
                       .arg(QString::fromStdString(s.classesDescription))
                       .arg(QString::fromStdString(s.detailDescription));
        if (i + 1 < synergies.size()) {
            tooltip += "\n";
        }
    }
    synergyLabel_->setText(text.trimmed());
    synergyLabel_->setToolTip(tooltip.trimmed());
}

void QtMainWindow::updateShopDisplay() {
    if (shopPanel_ != nullptr) {
        shopPanel_->updateDisplay(shop_, player_.gold());
    }
}

// 流程：清空旧道具按钮 ──> 无道具则占位 ──> 为每件 pending 创建装备按钮并绑定
void QtMainWindow::updateItemsDisplay() {
    if (itemsLayout_ == nullptr) {
        return;
    }

    // 清除旧按钮
    while (itemsLayout_->count() > 0) {
        QLayoutItem* item = itemsLayout_->takeAt(0);
        if (item->widget() != nullptr) {
            delete item->widget();
        }
        delete item;
    }

    if (pendingItems_.empty()) {
        QLabel* empty = new QLabel("（暂无道具）", itemsWidget_);
        empty->setStyleSheet("color: #6C7086; font-size: 11px;");
        itemsLayout_->addWidget(empty);
        return;
    }

    bool canEquip = fsm_.canPlayerAct() && (currentSelectedUnitId_ >= 0) &&
                    (unitsMap_.find(currentSelectedUnitId_) != unitsMap_.end()) &&
                    (unitsMap_.at(currentSelectedUnitId_)->owner() == core::UnitOwner::player);
    if (canEquip) {
        const core::Unit* selected = unitsMap_.at(currentSelectedUnitId_);
        if (static_cast<int>(selected->equippedItems().size()) >= selected->equipSlotCount()) {
            canEquip = false;
        }
    }

    for (int i = 0; i < static_cast<int>(pendingItems_.size()); ++i) {
        const core::ItemDef& def = core::getItemDef(pendingItems_.at(static_cast<std::size_t>(i)));
        QPushButton* btn = new QPushButton(
            QString("装备 %1").arg(QString::fromStdString(def.name)), itemsWidget_);
        btn->setStyleSheet(
            "QPushButton { background-color: #313244; color: #FAB387; font-size: 11px;"
            " border-radius: 3px; padding: 3px 6px; }"
            "QPushButton:disabled { background-color: #1E1E2E; color: #6C7086; }");
        btn->setEnabled(canEquip);
        btn->setToolTip(QString("物攻+%1%%  法攻+%2%%  物防+%3%%  魔防+%4%%  生命+%5%%  攻速+%6%%  法力%7")
            .arg(def.bonusPhysAtkPercent).arg(def.bonusMagAtkPercent)
            .arg(def.bonusPhysDefensePercent).arg(def.bonusMagDefensePercent)
            .arg(def.bonusMaxHpPercent).arg(def.bonusAttackSpeedPercent)
            .arg(def.bonusMaxManaFlat == 0
                     ? QString::fromUtf8("不变")
                     : QString("%1%2").arg(def.bonusMaxManaFlat > 0 ? "+" : "")
                           .arg(def.bonusMaxManaFlat)));
        btn->setProperty("itemIndex", i);
        connect(btn, SIGNAL(clicked()), this, SLOT(onEquipItem()));
        itemsLayout_->addWidget(btn);
    }
}

// 流程：取 pending 道具 ──> 校验选中英雄与槽位 ──> equipItem ──> 从 pending 移除并刷新 UI
void QtMainWindow::onEquipItem() {
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    if (btn == nullptr) {
        return;
    }
    const int idx = btn->property("itemIndex").toInt();
    if (idx < 0 || idx >= static_cast<int>(pendingItems_.size())) {
        return;
    }
    if (currentSelectedUnitId_ < 0) {
        statusBar()->showMessage("请先点击选中一个己方英雄再装备道具", 2000);
        return;
    }
    std::map<int, core::Unit*>::iterator it = unitsMap_.find(currentSelectedUnitId_);
    if (it == unitsMap_.end() || it->second == nullptr ||
        it->second->owner() != core::UnitOwner::player) {
        statusBar()->showMessage("请选中己方英雄后再装备", 2000);
        return;
    }

    core::Unit* unit = it->second;
    if (static_cast<int>(unit->equippedItems().size()) >= unit->equipSlotCount()) {
        statusBar()->showMessage(
            QString("装备槽已满（%1/%2），请先卸下一件装备")
                .arg(unit->equippedItems().size())
                .arg(unit->equipSlotCount()),
            2000);
        return;
    }
    const core::ItemType item = pendingItems_.at(static_cast<std::size_t>(idx));
    unit->equipItem(item);
    pendingItems_.erase(pendingItems_.begin() + idx);

    // 刷新显示
    refreshPreparationSynergyBuffs();
    updateItemsDisplay();
    updateStatusPanel();

    const core::ItemDef& def = core::getItemDef(item);
    statusBar()->showMessage(
        QString("%1 装备了 %2")
            .arg(QString::fromStdString(unit->name()))
            .arg(QString::fromStdString(def.name)),
        2500);
}

// 流程：校验阶段与槽位 ──> unequipItemAt ──> 道具归还 pendingItems_ ──> 刷新面板
void QtMainWindow::onUnequipItem(int unitId, int slotIndex) {
    if (!fsm_.canPlayerAct()) {
        statusBar()->showMessage("战斗阶段无法卸除装备", 1500);
        return;
    }
    std::map<int, core::Unit*>::iterator it = unitsMap_.find(unitId);
    if (it == unitsMap_.end() || it->second == nullptr ||
        it->second->owner() != core::UnitOwner::player) {
        return;
    }
    core::Unit* unit = it->second;
    if (slotIndex < 0 || slotIndex >= static_cast<int>(unit->equippedItems().size())) {
        return;
    }
    const core::ItemType item = unit->equippedItems().at(static_cast<std::size_t>(slotIndex));
    unit->unequipItemAt(slotIndex);
    pendingItems_.push_back(item);

    refreshPreparationSynergyBuffs();
    updateItemsDisplay();
    updateStatusPanel();

    const core::ItemDef& def = core::getItemDef(item);
    statusBar()->showMessage(
        QString("%1 卸下了槽%2的 %3（已归还到待装备列表）")
            .arg(QString::fromStdString(unit->name()))
            .arg(slotIndex + 1)
            .arg(QString::fromStdString(def.name)),
        2000);
}

}  // namespace ui
}  // namespace my_auto_arena
