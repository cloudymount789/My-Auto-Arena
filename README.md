# My-Auto-Arena

`My-Auto-Arena` 是 `Synera: Synergy Auto-Arena` 的 C++ 课程项目实现。  
当前版本已完成 **Phase 1（棋盘、备战区、拖拽、Qt GUI）** 与 **Phase 2（FSM、战斗引擎、BFS 寻路、5 英雄技能、PvE 关卡推进）**，正在推进 Phase 3 扩展功能。

## File Tree

```text
.
├── CMakeLists.txt
├── src
│   ├── main.cpp                   # 控制台演示入口
│   ├── qt_main.cpp                # Qt GUI 入口
│   ├── core
│   │   ├── Tile.h / Tile.cpp              # 单格占用模型
│   │   ├── Board.h / Board.cpp            # 棋盘与备战区管理
│   │   ├── Unit.h / Unit.cpp              # 单位基类 + WarriorUnit/MageUnit
│   │   ├── Player.h / Player.cpp          # 玩家全局状态
│   │   ├── DragDropHandler.h / .cpp       # 拖拽逻辑层
│   │   ├── GameFSM.h / GameFSM.cpp        # 三阶段状态机
│   │   ├── BattleEngine.h / .cpp          # 战斗 Tick 驱动（含移动）
│   │   ├── Pathfinder.h / Pathfinder.cpp  # BFS 网格寻路
│   │   ├── HeroUnits.h / HeroUnits.cpp    # 5 种英雄技能（多态）
│   │   ├── EnemySpawner.h / .cpp          # PvE 敌方生成器
│   │   └── PvERoundRunner.h / .cpp        # 单轮 PvE 推进与结算
│   └── ui
│       ├── ConsoleRenderer.h / .cpp       # 控制台渲染器
│       └── qt
│           ├── SceneCoordMapper.h / .cpp  # 像素↔逻辑坐标映射
│           ├── TileGraphicsItem.h / .cpp  # 格子图元
│           ├── UnitGraphicsItem.h / .cpp  # 单位图元（含鼠标拖拽）
│           ├── ArenaScene.h / ArenaScene.cpp  # 场景容器与拖拽结果处理
│           ├── UnitInfoPanel.h / .cpp     # 单位属性面板
│           └── QtMainWindow.h / .cpp      # 主窗口拼装
├── tests
│   ├── CMakeLists.txt
│   ├── test_board.cpp
│   ├── test_unit.cpp
│   ├── test_player.cpp
│   ├── test_drag_drop.cpp
│   ├── test_console_renderer.cpp
│   ├── test_scene_coord_mapper.cpp
│   ├── test_game_fsm.cpp
│   ├── test_battle_engine.cpp
│   ├── test_enemy_spawner.cpp
│   ├── test_pathfinder.cpp
│   ├── test_hero_skills.cpp
│   └── test_pve_round_runner.cpp
└── docs
    ├── development-plan.md
    ├── dev_log.md
    ├── code_analysis.md
    └── gui_acceptance_phase2.md   # Phase 1+2 GUI 验收操作指南
```

## 构建与运行

### 核心逻辑与测试（不需要 Qt）

```bash
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build --output-on-failure
```

### Qt GUI

1. 配置工程：`cmake -S . -B build`（若找不到 Qt，请设置 `CMAKE_PREFIX_PATH` 指向 Qt 6 的 MSVC kit，或将 `Qt\bin` 加入 `PATH`）。
2. 编译 GUI target：`cmake --build build --config Debug --target my_auto_arena_qt`
3. 运行：`build\Debug\my_auto_arena_qt.exe`（若缺 DLL，执行 `windeployqt build\Debug\my_auto_arena_qt.exe`）。

> 棋盘拖拽由 `UnitGraphicsItem` 捕获鼠标事件 → `ArenaScene` 调用 `DragDropHandler::execute` 执行逻辑放置；详见 `docs/code_analysis.md`。

## Core Classes and Data Structures

### Phase 1（棋盘与交互层）

| 类 | 职责 |
|---|---|
| `Tile` | 表示单个棋盘格，维护坐标与占用状态（`place / clear / occupied`）。 |
| `Board` | 维护 M×N 棋盘网格与备战区槽位；提供放置、清空、占用查询、半场判定、单位坐标检索接口。 |
| `Unit` | 单位基类，封装通用战斗属性（`hp/maxHp/attack/attackRange/mana/maxMana`）与基础行为；声明 `castFullManaSkill` 多态接口。 |
| `WarriorUnit` / `MageUnit` | Phase 1 两个演示子类，用于验证继承结构与测试流程。 |
| `Player` | 玩家全局状态：`gold / hp / level / populationCap / unitIds`；提供 `populationOnBoard` 统计上阵人口。 |
| `DragDropHandler` | 处理"备战区 ↔ 棋盘"拖拽逻辑；支持空位放置、格间交换、非法目标回弹、原地无操作；扩展版含半场与人口约束。 |
| `ConsoleRenderer` | 阶段一终端渲染器：输出棋盘、备战区与单位属性面板，与 Qt GUI 并行作为基础验收工具。 |
| `SceneCoordMapper` | 像素坐标 ↔ 逻辑格子坐标的双向映射，不依赖 Qt，可独立单元测试。 |
| `TileGraphicsItem` | Qt 格子图元；不接受鼠标输入（`Qt::NoButton`），避免遮挡单位拖拽区域。 |
| `UnitGraphicsItem` | Qt 单位图元；处理 `mousePressEvent / mouseMoveEvent / mouseReleaseEvent`，拖拽期间跟随光标移动，释放时发出信号。 |
| `ArenaScene` | Qt 场景容器；接收单位图元拖拽信号，调用 `DragDropHandler::execute`，成功则吸附格心，失败则动画回弹。 |
| `UnitInfoPanel` | Qt 侧栏面板，显示选中单位的 HP/Mana/ATK/Range。 |
| `QtMainWindow` | 主窗口：拼装 `ArenaScene`（左）+ `UnitInfoPanel`（右），状态栏显示操作结果。 |

### Phase 2（战斗系统）

| 类 | 职责 |
|---|---|
| `GameFSM` | 三阶段状态机：`kPrepare → kBattle → kSettlement → kPrepare`；维护 `currentRound`、`RoundOutcome`，提供 `canPlayerAct()` 门控。 |
| `BattleEngine` | 战斗 Tick 驱动：每帧执行索敌→移动→攻击（满蓝施法）→死亡清理→胜负判定；超时兜底。 |
| `Pathfinder` | BFS 网格寻路：从当前格出发，找到能对目标单位发动攻击的最近可用格，返回路径第一步。 |
| `AshRaiderHero` | 技能：对主目标造成 180 点爆发伤害。 |
| `NightArcherHero` | 技能：对主目标造成 210 点穿透伤害（远程）。 |
| `CurseHammerHero` | 技能：对自身周围 4 个相邻格中的敌方单位各造成 120 点伤害（AOE）。 |
| `MistWitchHero` | 技能：对主目标造成 90 点法术伤害（中程）。 |
| `BonePrayerHero` | 技能：为血量最低的友方治疗 150 点；若无其他友方则自愈 120 点。 |
| `EnemySpawner` | PvE 敌方生成器：按轮次静态配置生成敌方单位并部署到敌方半场。 |
| `PvERoundRunner` | 单轮 PvE 完整流程：生成敌军 → 驱动 BattleEngine → 结算金币/扣血 → 清理敌方指针。 |

## Algorithm Notes

### 格子放置校验（Grid Placement Validation）
所有放置操作先做边界检查（`Board::inBounds`），再做占用检查（`occupantOnBoard != kEmptySlot`）；越界返回 `kOutOfBounds`，已占用执行交换。

### 拖拽结果分类（Drag-and-Drop Resolution）
`DragDropHandler::execute` 对拖拽结果统一分类：
- `kSuccess`：目标空位，直接移动
- `kSwapped`：目标有单位，双方互换
- `kInvalidSource`：起点无单位，回弹
- `kOutOfBounds`：目标越界，回弹
- `kSameLocation`：原地拖拽，无操作
- `kNotPlayerHalf`：目标在敌方半场，禁止
- `kPopulationFull`：上阵人口已满，禁止

### 战斗状态机（Game FSM）
合法转移路径：`Prepare → Battle → Settlement → Prepare`；非法转移返回 `kIllegalTransition`；游戏结束后所有转移返回 `kGameOver`。`canPlayerAct()` 仅在 `Prepare` 且未 GameOver 时返回 `true`，作为拖拽/商店门控。

### 战斗 Tick 流程（Battle Engine）
每 Tick：
1. 遍历所有存活单位
2. 选最近敌方目标（并列规则：低血量优先 → 列从左到右 → 行从下到上）
3. 若目标在攻击距离内：造成伤害并回蓝（满蓝施法）
4. 若目标不在攻击距离内：调用 Pathfinder 向目标移动一格
5. Tick 末统一清理死亡单位
6. 检测一方全灭或超时（最大 5000 Tick）

### BFS 寻路（Pathfinder）
从移动单位当前格出发，BFS 遍历上下左右四格（仅空格可通行，自身起始格可穿越），寻找欧氏距离 ≤ `attackRange` 且空闲的目标邻近格；找到后回溯路径，返回路径第一步坐标。不可达时返回 `false`，单位原地停留。

### 技能系统（Skills，多态）
`Unit::castFullManaSkill` 是虚函数，满蓝时由 `BattleEngine` 调用，各子类独立覆写：
- 单目标爆发（AshRaider、NightArcher、MistWitch）
- AOE 伤害（CurseHammer，4 相邻格）
- 治疗（BonePrayer，最低血友方）

### PvE 关卡推进（PvERoundRunner）
`runRoundBattle` 封装完整一轮 PvE：`EnemySpawner::spawnRound` → `BattleEngine` 驱动至结束 → 按 `LevelConfig` 结算金币/扣血 → `removeEnemyUnits` 清理指针。

## Helper Functions

- `Board::findUnitOnBoard(unitId)`：在棋盘全局搜索并返回指定单位的 `Position`，用于技能与引擎中的坐标检索。
- `Board::isPlayerHalf / isEnemyHalf`：半场判定，用于拖拽约束与敌方部署。
- `Pathfinder::nextStepTowardAttackRange`：静态方法，不持有状态，便于测试与复用。
