# 开发日志

## 2026-04-22

- **Qt GUI 拖拽修复**：`UnitGraphicsItem` 在左键 `mousePressEvent` 中未 `accept()` 时，场景不会将该项作为 mouse grabber，导致收不到后续 `mouseMoveEvent`，表现为无法拖拽。已为左键按下/移动/释放补全 `accept()`，并在移动时校验 `buttons()` 仍含左键。
- **`TileGraphicsItem`**：构造函数中 `setAcceptedMouseButtons(Qt::NoButton)`，避免格子在视觉上大于单位 `boundingRect` 的留白区域抢走点击，从而无法把拖拽交给上层的 `UnitGraphicsItem`。

## Step 1 - 按规则重启实现
- 明确采用 `docs/development-plan.md` 的阶段一目标：仅保留“工程初始化 + Board/Tile/Unit + 基础测试”。
- 按 `zh-development-workflow.mdc` 要求执行：
  - 先读取规划文档
  - 使用 ECC `planner` 代理生成本步实现计划
- 按 `cpp-coding-style.mdc` 执行风格约束：
  - 回退到 C++11/14 教学风格
  - 不使用 `std::optional`、`[[nodiscard]]` 等高级写法
  - 统一类名 PascalCase、函数/变量 camelCase

## Step 2 - 清除偏离计划的实现
- 删除越界到 Phase 2 的文件：
  - `src/core/GameFsm.h`
  - `src/core/GameFsm.cpp`
  - `tests/test_game_fsm.cpp`
- 清理构建残留目录：
  - `build/`
  - `cmake-build-debug/`
  - `cmake-build-release/`

## Step 3 - 从头重写阶段一骨架
- 重写 `CMakeLists.txt`：
  - `CMAKE_CXX_STANDARD` 设为 14
  - 移除 `GameFsm.cpp`
- 重写核心模型：
  - `src/core/Tile.*`
  - `src/core/Board.*`
  - `src/core/Unit.*`
- 重写测试：
  - `tests/test_board.cpp`
  - `tests/test_unit.cpp`
  - `tests/CMakeLists.txt`（移除 `test_game_fsm.cpp`）

## Step 4 - 文档重写与进度同步
- 从头重写 `README.md`，只保留当前阶段实现内容。
- 从头重写 `docs/dev_log.md` 与 `docs/code_analysis.md`。
- 在 `docs/development-plan.md` 标注当前已完成部分（阶段一前 3 步）。
## Step 5 - 重构后审查与修正
- 使用 ECC `code-reviewer` 对重构结果进行了审查。
- 已修正项：
  - `Unit` 析构函数改为 `= default` 写法（保留教学风格中的 `virtual + override`）。
  - `Board::inBounds` 声明与定义参数签名对齐。
  - 在 `Unit` 构造中补充注释说明 `attack == 0` 的设计意图。
- 保留项（有意）：
  - `Tile/Board/Unit` 的拷贝构造函数保持显式定义，以符合课程规则“显式定义拷贝构造函数”。

## Step 6 - 完成 Phase 1 第4和5步
- 按 `zh-development-workflow.mdc` 要求先读取 `docs/development-plan.md`，并使用 ECC `planner` 代理生成本步计划。
- 新增拖拽交互逻辑（Phase 1 Step 4）：
  - `src/core/DragDropHandler.h`
  - `src/core/DragDropHandler.cpp`
  - 支持：备战区->棋盘、棋盘->备战区、棋盘内移动、交换、非法目标回弹、原地无操作
- 新增基础 GUI 呈现（Phase 1 Step 5）：
  - `src/ui/ConsoleRenderer.h`
  - `src/ui/ConsoleRenderer.cpp`
  - 渲染：棋盘、备战区、单位属性面板（HP/Mana/ATK/Range）
- 更新测试：
  - `tests/test_drag_drop.cpp`
  - `tests/test_console_renderer.cpp`
  - `tests/CMakeLists.txt` 增加新测试
- 更新主程序演示：
  - `src/main.cpp` 增加拖拽与渲染 demo
- 构建验证：
  - `cmake --build build`
  - `ctest --test-dir build --output-on-failure`
  - 结果：21/21 测试通过
- 标注计划进度：
  - `docs/development-plan.md` 已将 Phase 1 的第4、5步标记为完成。

## Step 7 - 修复需求对齐缺口（当前可修复部分）
- 新增 `Player` 全局实体：
  - `src/core/Player.h`
  - `src/core/Player.cpp`
  - 属性：血量、金币、等级、人口上限、拥有单位集合
- 补充棋盘半场判定：
  - `Board::isPlayerHalf`
  - `Board::isEnemyHalf`
- 强化拖拽约束（启用玩家上下文时）：
  - 禁止拖拽到敌方半场（返回 `kNotPlayerHalf`）
  - 达到人口上限时禁止备战区上阵（返回 `kPopulationFull`）
  - 校验拖拽源单位必须属于当前玩家
- 补充测试：
  - `tests/test_player.cpp`
  - `tests/test_board.cpp` 新增半场判定用例
  - `tests/test_drag_drop.cpp` 新增半场/人口/约束用例
- 构建与测试验证：
  - 全量 `ctest`：29/29 通过

## Step 8 - Qt GUI 最小可用版本
- 按 `docs/development-plan.md` 的“Phase 1 Step 5 基础 GUI 呈现”继续推进到图形窗口实现。
- 新增可选 Qt6 构建路径（不阻断核心构建）：
  - `CMakeLists.txt` 增加 `BUILD_QT_GUI` 选项
  - `find_package(Qt6 COMPONENTS Widgets QUIET)`，未安装 Qt 时仅跳过 GUI target
- 新增图形层文件：
  - `src/ui/qt/SceneCoordMapper.*`
  - `src/ui/qt/TileGraphicsItem.*`
  - `src/ui/qt/UnitGraphicsItem.*`
  - `src/ui/qt/ArenaScene.*`
  - `src/ui/qt/UnitInfoPanel.*`
  - `src/ui/qt/QtMainWindow.*`
  - `src/qt_main.cpp`
- 拖拽流程：
  - 鼠标拖放释放后调用 `DragDropHandler::execute`
  - 成功时吸附到格子中心
  - 非法放置（越界/半场/人口）时做动画回弹
- 测试补充：
  - `tests/test_scene_coord_mapper.cpp`
  - `tests/CMakeLists.txt` 接入该测试
- 验证结果：
  - 本机未检测到 Qt6 时，GUI target 被安全跳过
  - 核心测试保持通过：`ctest` 33/33 通过

## Step 9 - 进入 Phase 2：游戏状态机（Step 1）
- 按 `development-plan` 与 `requirement` 的 Phase2 范围，仅实现 `Prepare/Battle/Settlement` 三阶段状态机，不进入战斗引擎与寻路。
- 新增：
  - `src/core/GameFSM.h`
  - `src/core/GameFSM.cpp`
  - `tests/test_game_fsm.cpp`
- 核心能力：
  - 合法流转：`Prepare -> Battle -> Settlement -> Prepare`
  - 非法流转结果区分：`kAlreadyInPhase / kIllegalTransition`
  - `setGameOver()` 后统一阻断流转：`kGameOver`
  - `canPlayerAct()` 用于后续拖拽/商店门控
  - `RoundOutcome` + `hasOutcome()` 用于结算结果追踪
- 构建接入：
  - `CMakeLists.txt` 将 `GameFSM.cpp` 编入 `game_core`
  - `tests/CMakeLists.txt` 将 `test_game_fsm.cpp` 编入 `core_tests`
- 测试结果：
  - 新增 FSM 测试 7 个用例通过
  - 全量 `ctest`：40/40 通过

## Step 10 - Phase 2 Step 2：战斗引擎 Tick 循环
- 新增战斗引擎：
  - `src/core/BattleEngine.h`
  - `src/core/BattleEngine.cpp`
- 新增敌方生成器（对接 `src/design/enemy_roster_and_initial_levels.md` 的最小轮次）：
  - `src/core/EnemySpawner.h`
  - `src/core/EnemySpawner.cpp`
- 扩展棋盘查询能力：
  - `Board::findUnitOnBoard(unitId)` 用于战斗中坐标检索
- 关键行为：
  - 每 tick 执行“索敌 -> 射程判定 -> 攻击 -> 回蓝”
  - tick 末清理死亡单位
  - 一方全灭结算胜负
  - 超过最大 tick 上限触发超时结算
- 新增测试：
  - `tests/test_battle_engine.cpp`
  - `tests/test_enemy_spawner.cpp`
- 审查后修复：
  - 删除 `BattleEngine` 拷贝构造（避免共享引用误用）
  - 死亡单位从 `units_` 注册表中移除，避免后续状态污染
  - 在索敌并列规则处补充注释说明
- 测试结果：
  - 全量 `ctest`：47/47 通过

## Step 11 - Phase 2 Step 3：路径与碰撞处理（Pathfinder BFS）
- 新增寻路模块：
  - `src/core/Pathfinder.h`
  - `src/core/Pathfinder.cpp`
- 核心能力：
  - BFS 网格寻路：从移动单位当前格出发，寻找欧氏距离 ≤ `attackRange` 的可攻击空格
  - 路径回溯：找到目标格后沿父节点链反向追踪，返回路径第一步坐标（`outNext`）
  - 不可达兜底：BFS 队列耗尽未找到目标格时返回 `false`，单位原地停留，不死循环
  - 已占用格阻挡：只允许空格通行；自身起始格不阻挡（支持单位"离开"当前格的路径规划）
  - 接入 `BattleEngine`：当目标不在攻击范围内时调用 `Pathfinder::nextStepTowardAttackRange`，移动单位一步后更新 Board 占用状态
- 新增测试：
  - `tests/test_pathfinder.cpp`
- 构建与测试验证：全量 `ctest` 通过

## Step 12 - Phase 2 Step 4：技能与法力系统（5 种英雄，多态）
- 在 `Unit` 基类新增接口：
  - `virtual castFullManaSkill(Board&, map<int,Unit*>&, Unit* primaryTarget)`：法力满时由引擎调用
  - `heal(amount)`：血量恢复，不超过 `maxHp`
  - `spendAllMana()`（protected）：技能释放后清空法力
- 新增英雄实现：
  - `src/core/HeroUnits.h`
  - `src/core/HeroUnits.cpp`
  - 共 5 种英雄子类，覆写 `castFullManaSkill`：
    - `AshRaiderHero`（灰烬掠袭者）：单目标 180 爆发
    - `NightArcherHero`（夜羽猎弓手）：单目标 210 远程
    - `CurseHammerHero`（诅印重锤奴）：AOE 4 邻格各 120
    - `MistWitchHero`（瘴雾魔女学徒）：单目标 90 法术
    - `BonePrayerHero`（骨契祷告者）：治疗最低血友方 150，无友方则自愈 120
- 接入 `BattleEngine`：每次攻击后 `gainMana`，法力满时调用 `castFullManaSkill` 并传入主目标
- 新增测试：
  - `tests/test_hero_skills.cpp`：覆盖 5 种技能触发效果
- 构建与测试验证：全量 `ctest` 通过

## Step 13 - Phase 2 Step 5：PvE 关卡推进与结算
- 新增 PvE 关卡推进器：
  - `src/core/PvERoundRunner.h`
  - `src/core/PvERoundRunner.cpp`
- 核心能力：
  - `runRoundBattle`：封装完整一轮 PvE 流程——生成敌军 → 驱动 BattleEngine 至结束 → 金币/扣血结算 → 清理敌方指针
  - `removeEnemyUnits`：战后从 Board 和 units 注册表中彻底删除敌方单位，防止内存泄漏与状态污染
  - 与 `GameFSM` 配合：`RoundOutcome` 由 BattleEngine 产出，再由 FSM 的 `startSettlement` 接收
- 新增测试：
  - `tests/test_pve_round_runner.cpp`
- 构建与测试验证：全量 `ctest` 通过，Phase 2 全部 5 步闭环完成

## Step 14 - 代码审查修复（code-reviewer 全面审查后执行）

### Critical / High 修复
- **C-1（内存泄漏）**：`BattleEngine::clearDeadUnits` 移除死亡单位时不具备指针所有权，不在此删除；  
  改由 `PvERoundRunner::runRoundBattle` 在战斗后统一处理：战中已死亡（被 `clearDeadUnits` 移除出 map）的敌方单位由 Runner 补充 `delete`，存活敌方仍由 `removeEnemyUnits` 清理，实现完整内存回收。
- **H-1/H-2**：`ArenaScene.cpp` 和 `QtMainWindow.cpp` 中所有 `vector[]` 访问改为 `.at()`，符合项目规则。
- **H-3**：`DragDropHandler` 交换回滚路径中补充对 `placeUnit` 返回值的捕获（`restoredSrc / restoredTgt`）并加注释，消除静默状态损坏风险。

### Medium 修复
- **M-1**：`GameFSM::lastOutcome()` 增加前置守卫，`hasOutcome_ == false` 时抛出 `std::logic_error`，防止误读默认值。
- **M-2**：`RoundOutcome` 新增 `bool gameOver` 字段；`PvERoundRunner` 在玩家 HP 归零时设置为 `true`，调用方可据此触发 `GameFSM::setGameOver()`。
- **M-3**：`WarriorUnit`、`MageUnit`、`AshRaiderHero`、`NightArcherHero`、`CurseHammerHero`、`MistWitchHero`、`BonePrayerHero`、`SpawnedEnemyUnit` 全部补充显式拷贝构造函数（委托给 `Unit(other)`），符合课程规范。
- **M-4**：`BattleEngine` 补充 `operator=(const BattleEngine&) = delete`，完整表达"禁止拷贝"意图（Rule of Three/Five）。
- **M-5**：`Unit` 补充 `operator=(const Unit&)` 实现，配套显式拷贝构造函数满足 Rule of Three。

### Low 修复
- **L-1（重复代码）**：`Unit` 新增 `protected` 辅助方法 `performAttackInRange`；`WarriorUnit` 和 `MageUnit` 的 `castFullManaSkill` 改为调用该方法，消除完全相同的实现重复。
- **L-2**：`Pathfinder::nextStepTowardAttackRange` 的 `units` 参数加注释"预留参数：用于未来按单位类型设置通行性"，并补充 `(void)movingUnitId`。
- **L-3（死代码）**：`Pathfinder` 中 `canEnterCell` 的"起始格特判"分支（永远不可达）已删除，简化为单纯空格判断，并补充注释说明原因。
- **L-4**：`EnemySpawner` 的 no-op 拷贝构造函数改为 `= default`（等价且明确）；`SpawnedEnemyUnit` 补充显式拷贝构造函数。
- **L-5**：`EnemySpawner::configForRound` 在 `round` 超出 `[1..6]` 时抛出 `std::out_of_range`，消除静默返回空配置的问题；对应测试 `InvalidRoundSpawnsEmpty` 更名为 `InvalidRoundThrowsOutOfRange` 并改用 `EXPECT_THROW`。
- **L-6（冗余查询）**：`BattleEngine::selectTarget` 平局判定中，`bestPos` 随 `best` 一同更新缓存，避免每次并列比较都重新执行 O(rows×cols) 的 `findUnitOnBoard` 查询。

### 验证结果
- 编译：零错误零警告
- `ctest`：**53/53 测试通过**
- 窗口标题更新为"Synera: Synergy Auto-Arena (Phase 1 + Phase 2)"

## Phase 2 完成总结
- 阶段二全部 5 步通过验收：FSM + BattleEngine（含移动）+ BFS 寻路 + 5 英雄技能（多态）+ PvE 关卡推进
- README.md、docs/code_analysis.md、docs/dev_log.md 同步更新，与代码实现对齐
- 生成 `docs/gui_acceptance_phase2.md` 作为 Phase 1+2 GUI 验收操作指南

---

## 2026-05-20 — Phase 3 实现

### 新增核心系统

**Item 系统**
- 定义 `ItemType` 枚举（kNone/kSword/kArmor/kRing/kTalisman）
- `ItemDef` 结构体含 name/bonusAtk/bonusMaxHp
- `getItemDef()` 静态表工厂

**Unit 扩展**
- 新增字段：`unitClass_`（职业）、`starLevel_`（1-3）、`equippedItem_`、`bonusAtk_`/`bonusMaxHp_`（羁绊临时加成）、`star1Atk_`/`star1MaxHp_`（原始星级1基础值，用于升星乘算）
- `attack()` / `maxHp()` 改为返回含羁绊加成的有效值
- `equipItem` / `unequipItem`：装备加成直接叠加到 `attack_`/`maxHp_`，卸装时减去
- `setSynergyBuffs` / `clearSynergyBuffs`：羁绊 BUFF 管理
- `upgradeToStar`：用 star1 基础值 × 倍率（star2=1.8, star3=3.0），保留装备加成
- `resetToFull` 更新为含羁绊加成的满血

**HeroUnits**
- 5 英雄构造函数增加 `UnitClass` 参数
- 新增 `HeroType` 枚举与 `createHero()` 工厂函数

**Shop 系统**
- 5 槽位，kHeroCost=3，kRefreshCost=2
- `buy()` 创建英雄并扣金；`refresh()` 随机重置所有槽位
- `sellValue()` 静态方法：star1→1金，star2→2金，star3→4金

**SynergySystem**
- 4 种羁绊：近战（战士+坦克，2→+300HP，4→+700HP）、弓手（2→+50ATK，3→+120ATK）、法术（1→+70ATK，2→+160ATK）、圣愈（1→+400HP，2→+900HP）
- `applyBuffs`：遍历棋盘玩家单位并调用 `setSynergyBuffs`
- `clearBuffs`：结算后清零所有玩家单位羁绊加成
- `getActiveSynergies`：返回 UI 展示用的激活羁绊列表

**StarUpgrade**
- `tryMergeAll`：循环扫描玩家单位，发现 3 张同名同星级时自动合并，删除 2 张并对保留单位调用 `upgradeToStar`
- `removeUnit`：从棋盘/备战区/玩家列表/unitsMap 完整清除并释放内存

**SaveManager**
- 文本格式 key=value 存档
- 保存：轮次、玩家HP/金/人口上限、每英雄的位置/星级/装备、待装备道具列表
- 读档：还原所有状态并重建 Unit 指针
- try-catch 保护文件 I/O

### Qt GUI 集成

**ShopPanel**
- 5 个英雄购买槽按钮（显示英雄名+费用或"已售出"）
- 刷新按钮；`updateDisplay()` 根据金币状态控制按钮启用
- 信号：`heroPurchased(int)` / `refreshRequested()`

**UnitGraphicsItem**
- 新增 `starLevel_` 字段，在名称下方绘制黄色 ★ 符号

**UnitInfoPanel**
- 新增职业/星级/装备标签；出售按钮（sellRequested 信号）
- `currentUnitId_` 跟踪当前选中单位

**QtMainWindow**
- 新增：商店面板、人口标签、羁绊标签、升级人口按钮、存档/读档按钮
- `onHeroPurchased`：购买→放备战区→升星检测
- `onShopRefresh`：扣金刷新商店
- `onSellUnit`：出售英雄（从棋盘/unitsMap/playerUnits 清除）
- `onLevelUp`：人口上限+1（费用=当前上限×2，最大8）
- `onSaveGame` / `onLoadGame`：文件对话框 + SaveManager
- `onStartBattle`：战斗前调用 `SynergySystem::applyBuffs`
- `doSettlement`：战斗后 `clearBuffs` + 升星检测 + 随机道具掉落（轮次≥2）+ 刷新商店

### 测试覆盖
- `test_shop.cpp`：14 个测试（购买/刷新/出售/金币/拷贝）
- `test_synergy.cpp`：11 个测试（各羁绊阈值/clearBuffs/getActiveSynergies）
- `test_star_upgrade.cpp`：12 个测试（upgradeToStar 倍率/保留装备/3合1/装备增减）
- `test_save.cpp`：4 个测试（存读档完整流程/无效路径/星级还原）
- **总计：94/94 测试通过**（原 53 + 新增 41）

## Phase 3 完成总结
- 阶段三全部功能实现：商店购买/刷新、人口升级、羁绊系统、升星合并、装备增减、存读档
- GUI 完整可玩：商店面板、羁绊展示、信息面板出售、星级显示、控制栏扩展
- development-plan.md 阶段三成功标准全部标记为已完成

## Bug 修复 + 数值策划重设计（2026-05-22）

### Bug 修复

#### BUG-1：升星崩溃（use-after-free）
- **根因**：`onHeroPurchased()` 在 `tryMergeAll()` 之后仍访问 `hero->name()`。若新购英雄是合并中被删除的两张之一，指针已失效，访问即崩溃。
- **修复**：在调用 `tryMergeAll` 之前将 `hero->name()` 存入局部 `std::string heroName`，此后不再访问 `hero` 指针。

#### BUG-2：装备血量未按比例放大
- **根因**：`Unit::equipItem()` 仅递增 `maxHp_`，`hp_` 不变，导致穿了加最大血量的装备（如锁甲）后角色显示"受伤"。
- **修复**：若装备 `bonusMaxHp > 0`，按照旧 maxHp/新 maxHp 的比值等比缩放 `hp_`，结果向上取整，最低保留1点血量。

#### BUG-3：升星后 UnitGraphicsItem 星级不更新
- **根因**：`ArenaScene::syncAfterBattle` 仅调用 `setStats(hp, maxHp, mana, maxMana)`，不刷新星级显示。
- **修复**：在 `syncAfterBattle` 的单位刷新循环中增加 `itemIt->second->setStarLevel(unit->starLevel())`。

#### BUG-4：出售后 currentSelectedUnitId_ 未归零
- **根因**：`onSellUnit()` 调用 `infoPanel_->setUnit(nullptr)` 清空面板，但 `currentSelectedUnitId_` 仍保留旧ID，后续装备操作可能误命中已释放内存。
- **修复**：在 `delete unit` 前将 `currentSelectedUnitId_ = -1`。

### 策划数值重设计

#### 设计目标
1. 玩家从第3关起感受到真实压力（旧版几乎不会输）。
2. 四条羁绊路线收益差异显著，迫使专攻而非随意混搭。
3. 升星和装备对局面产生质变影响。

#### 英雄数值（削弱血量、调整技能）
| 英雄 | 旧HP/ATK/技能 | 新HP/ATK/技能 |
|------|-------------|-------------|
| 战士 | 2200/60/300爆发 | 1600/62/280爆发 |
| 射手 | 1800/55/320穿透 | 1200/60/360穿透 |
| 重甲战士 | 3200/48/AOE180 | 2600/48/AOE220 |
| 法师 | 1500/35/350法术 | 1000/38/420法术 |
| 治疗师 | 1800/30/治疗500 | 1400/28/治疗600 |

**开局**：3英雄→2英雄（战士+射手），起始金币 10→8。迫使玩家用8金做第一次英雄选择。

#### 敌方配置（大幅增加数量和强度）
| 关卡 | 旧（数量） | 新（数量/组成） | 失败惩罚 |
|------|---------|--------------|--------|
| R1 | 1 | 2（战士×2） | 2→5 HP |
| R2 | 2 | 3（战士×2+射手） | 3→8 HP |
| R3 | 2 | 4（重甲+射手+战士+法师） | 4→12 HP |
| R4 | 3 | 5（★2重甲+射手×2+法师+治疗） | 5→15 HP |
| R5 | 3 | 5（★2重甲+★2射手+法师×2+治疗） | 6→18 HP |
| R6 | 4 | 6（★2攻城弩BOSS+重甲+战士×2+法师+治疗） | 7→25 HP |

#### 羁绊重设计（差异化收益）
| 羁绊 | 旧效果 | 新效果 | 策略定位 |
|-----|-------|-------|---------|
| 近战2 | 全体+300HP | 近战单位+45ATK | 近战爆发流 |
| 近战4 | 全体+700HP | 近战单位+110ATK+500HP | 坦克大前排 |
| 弓手2 | 弓手+50ATK | 弓手+100ATK | 远程输出流 |
| 弓手3 | 弓手+120ATK | 弓手+260ATK | 三射手秒杀流 |
| 法术1 | 法师+70ATK | 法师+120ATK | 技能爆发流 |
| 法术2 | 法师+160ATK | 法师+300ATK | 双法师质变 |
| 圣愈1 | 全体+400HP | 全体+800HP | 持久战辅助 |
| 圣愈2 | 全体+900HP | 全体+2000HP | 双治疗无敌 |

**测试：95/95 全部通过**

## 2026-05-22 – 续前次任务（星级加权羁绊 + 无尽关卡 + VFX 动画 + 装备穿脱）

### 星级加权羁绊系统（SynergySystem）
- `countClassOnBoard()` 改为累计星级点数（★1=1点，★2=2点，★3=3点）。
- 激活阈值改为 **T1=3点 / T2=6点**，取代旧的"单位数量 2/4"逻辑。
- 新羁绊数值：
  | 羁绊 | T1 (3pts) | T2 (6pts) |
  |-----|----------|----------|
  | 近战 | 近战单位 +70 ATK | 近战 +180 ATK +1000 HP |
  | 弓手 | 弓手 +160 ATK | 弓手 +400 ATK |
  | 法术 | 法师 +180 ATK | 法师 +450 ATK |
  | 圣愈 | 全体 +1000 HP | 全体 +2500 HP |
- `getActiveSynergies()` 未激活时显示"3/6 激活 (已有N点)"，便于玩家理解进度。

### 无尽关卡（EnemySpawner）
- 移除 `configForRound()` 的 `[1..6]` 范围校验，改为 clamp round < 1 为 1。
- 第7关起，阵型每3关循环（轻量/标准/精英），并按 `pow(1.12, round-6)` 对敌方 HP/ATK 指数膨胀。
- `LevelConfig` 新增 `statScaleFactor` 字段；`spawnRound()` 在构造敌方单位前先应用该系数。
- 失败惩罚上限 50 HP，金币奖励上限 30，随关数线性增加。

### VFX 动画系统（ArenaScene）
- 新增 `VfxProjectile` 结构体与 30ms `vfxTimer_` 实现弹体动画。
- 射手普通攻击：金黄色拉长椭圆，180ms 飞行，旋转指向目标。
- 法师普通攻击：深蓝紫色圆形，240ms 飞行。
- 技能弹体：按职业区分颜色（战士=血红，射手=亮金，坦克=橙，法师=深紫，治疗=翠绿）。
- 命中时生成白色闪光，坦克/治疗师技能改为静态范围光晕。

### 装备穿脱（UnitInfoPanel + QtMainWindow）
- `UnitInfoPanel` 新增「卸下装备」按钮，仅在玩家单位且已装备时启用。
- `QtMainWindow::onUnequipItem()` 调用 `unit->unequipItem()` 并将物品返还 `pendingItems_`。

### 备战阶段羁绊展示
- `onDragResult()` 每次拖拽结束后调用 `updateSynergyDisplay()`，使羁绊面板实时更新。

### 升星数值（Unit）
- ★2 倍率从 1.8x 改为 3.0x，★3 从 3.0x 改为 7.0x（相对★1基础值）。
- 新增 `scaledSkillDamage(int base)` 方法，使技能伤害同步受升星倍率影响。
- `HeroUnits.cpp` 各英雄 `castFullManaSkill()` 统一改用 `scaledSkillDamage()`。

**测试：95/95 全部通过**

---

## 2026-05-22（代码审查与边界条件修复）

本次对全部源码进行了一次系统性代码审查，共发现并修复 5 处 Bug。

### Bug 1（CRITICAL）：无尽关卡无法进行 — `QtMainWindow::onNextRound()`
- **位置**：`src/ui/qt/QtMainWindow.cpp`，`onNextRound()`
- **原因**：UI 层仍保留旧的 `if (fsm_.currentRound() >= 6)` 判断，弹出"已通过全部 6 关"并返回，完全阻断了第 7 关及之后的流程。`EnemySpawner` 虽已实现无尽膨胀逻辑，但 UI 层的硬编码上限使其从未被触及。
- **修复**：删除该判断块，`onNextRound()` 现在直接调用 `fsm_.startNextRound()` 无上限推进。

### Bug 2（HIGH）：卸除加血装备后 HP 溢出 — `Unit::unequipItem()`
- **位置**：`src/core/Unit.cpp`
- **原因**：`equipItem()` 在增加 `maxHp_` 时会按比例扩大 `hp_`（例如装锁甲后 HP 1600→2400）。但 `unequipItem()` 仅做了 `maxHp_ -= def.bonusMaxHp`，未将 `hp_` 钳制到新的上限，导致 `hp_ > maxHp()`，单位显示血量非法。
- **修复**：在减少 `maxHp_` 后追加 `hp_ = std::max(1, std::min(hp_, maxHp_))`。

### Bug 3（HIGH）：卸装备未检查战斗阶段 — `QtMainWindow::onUnequipItem()`
- **位置**：`src/ui/qt/QtMainWindow.cpp`
- **原因**：`onUnequipItem()` 缺少 `fsm_.canPlayerAct()` 前置检查，理论上可在战斗阶段触发卸装备，导致战斗中单位数值变化。
- **修复**：在函数入口添加阶段守卫，战斗期间调用时展示提示信息并提前返回。

### Bug 4（HIGH）：读档后棋盘位置静默失败 — `SaveManager::load()`
- **位置**：`src/core/SaveManager.cpp`
- **原因**：`load()` 清除旧玩家单位后未清空 `Board` 的格子占位数据。`Tile::place()` 在格子已占时返回 `false`，`Board::placeOnBench()` 同理，导致读档重建的英雄无法放置，读档后英雄消失。
- **修复**：在清除旧单位之后、放置新单位之前，用双层循环调用 `board.clearOnBoard()` / `board.clearOnBench()` 清空所有格子。

### Bug 5（MEDIUM）：商店英雄每次启动顺序相同 — `Shop::randomizeSlots()`
- **位置**：`src/qt_main.cpp`
- **原因**：`Shop::randomizeSlots()` 使用 `std::rand()` 但从未初始化随机种子，每次启动游戏商店英雄刷新序列完全一致。
- **修复**：在 `qt_main.cpp` 的 `main()` 入口调用 `std::srand(static_cast<unsigned int>(std::time(nullptr)))`。

**测试：95/95 全部通过**