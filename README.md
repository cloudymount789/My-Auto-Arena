# My-Auto-Arena

`My-Auto-Arena` 是 `Synera: Synergy Auto-Arena` 的 C++ 课程项目实现。
当前版本已按 C++11/14 教学风格重构，正在推进阶段一基础骨架。

## File Tree

```text
.
├── CMakeLists.txt
├── src
│   ├── main.cpp
│   ├── core
│       ├── Tile.h / Tile.cpp
│       ├── Board.h / Board.cpp
│       ├── Unit.h / Unit.cpp
│       ├── Player.h / Player.cpp
│       └── DragDropHandler.h / DragDropHandler.cpp
│   └── ui
│       ├── ConsoleRenderer.h / ConsoleRenderer.cpp
│       └── qt
│           ├── SceneCoordMapper.h / SceneCoordMapper.cpp
│           ├── TileGraphicsItem.h / TileGraphicsItem.cpp
│           ├── UnitGraphicsItem.h / UnitGraphicsItem.cpp
│           ├── ArenaScene.h / ArenaScene.cpp
│           ├── UnitInfoPanel.h / UnitInfoPanel.cpp
│           └── QtMainWindow.h / QtMainWindow.cpp
│   └── qt_main.cpp
├── tests
│   ├── CMakeLists.txt
│   ├── test_board.cpp
│   ├── test_unit.cpp
│   ├── test_player.cpp
│   ├── test_drag_drop.cpp
│   ├── test_console_renderer.cpp
│   └── test_scene_coord_mapper.cpp
└── docs
    ├── development-plan.md
    ├── dev_log.md
    └── code_analysis.md
```

## Core Classes and Data Structures

- `Tile`
  - 表示单个棋盘格，维护坐标与占用状态。
- `Board`
  - 维护棋盘网格与备战区槽位。
  - 提供放置、清空、占用查询接口。
- `Unit`
  - 单位基类，封装通用战斗属性与基础行为。
- `WarriorUnit`, `MageUnit`
  - 阶段一的两个示例子类，用于验证继承结构与测试流程。
- `DragDropHandler`
  - 处理“备战区 <-> 棋盘”的拖拽逻辑。
  - 支持空位放置、交换、非法目标回弹、原地无操作。
- `Player`
  - 玩家全局状态：金币、血量、等级、人口上限、拥有单位集合。
- `ConsoleRenderer`
  - 阶段一基础 GUI 的终端模拟渲染器。
  - 渲染棋盘、备战区与单位属性面板。
- `QtMainWindow` / `ArenaScene`
  - 阶段一 Qt 图形窗口实现：棋盘和备战区渲染、鼠标拖拽、非法回弹、单位信息面板。

## Algorithm Notes

- **Grid Placement Validation**
  - 所有放置操作先做边界检查，再做占用检查。
- **Occupancy Rule**
  - 一个棋盘格同一时刻最多容纳一个单位。
- **Bench Rule**
  - 一个备战槽位同一时刻最多容纳一个单位。
- **Pathfinding / Targeting / Synergy**
  - 规划在后续阶段实现：
    - 寻路：先 BFS，再视情况扩展 A*
    - 索敌：最近目标 + 平局规则
    - 羁绊：标签计数触发阈值 Buff
- **Drag-and-Drop Resolution**
  - `DragDropHandler::execute` 对拖拽结果进行分类：
    - 成功放置 `kSuccess`
    - 与目标交换 `kSwapped`
    - 起点无单位 `kInvalidSource`
    - 目标越界回弹 `kOutOfBounds`
    - 原地拖拽 `kSameLocation`
- **Base GUI Rendering**
  - `ConsoleRenderer` 按固定网格宽度输出：
    - 棋盘占位
    - 备战区槽位
    - 单位基础属性（HP/Mana/ATK/Range）
- **Qt Drag-and-Drop Rendering**
  - `ArenaScene` 将鼠标释放坐标映射为逻辑位置，并调用 `DragDropHandler`：
    - 合法放置：吸附到目标格
    - 非法放置：动画回弹

## Helper Functions

- 当前未引入全局工具函数。
- 工具逻辑优先放在类成员中，便于保持封装与不变量约束。