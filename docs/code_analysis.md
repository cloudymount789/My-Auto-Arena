# 代码解析（验收参考）

## Qt 图形项鼠标拖拽（`UnitGraphicsItem` / `TileGraphicsItem`）

- **事件流**：`QGraphicsView` 将鼠标事件交给场景，场景再分发给**最上层且声明接受对应鼠标键**的图元。左键按下后，只有被 `accept()` 的图元会成为 grabber，从而在按住移动时持续收到 `mouseMoveEvent`。
- **本次问题根因**：子类在 `mousePressEvent` 末尾调用基类默认实现时，事件可能未被接受，grabber 未建立，拖拽链断裂。
- **格子与单位叠放**：单位 `zValue` 高于格子，但单位绘制区域小于一格；若格子仍接受左键，则点击留白时由格子接收事件，单位无法开始拖拽。格子改为不接受任何鼠标键后，该区域的命中会穿透到下层；若该格有单位且点击落在单位矩形内，仍由单位处理。


本次重构目标是把工程严格拉回到两条主线：
1. **实现顺序对齐 `docs/development-plan.md`**：只完成阶段一前半（工程初始化 + 棋盘/备战区模型 + 单位基础模型 + 对应测试）。
2. **语法风格对齐 `cpp-coding-style.mdc`**：采用 C++11/14 教学风格，避免不必要的高级语法。
因此移除了不在当前步骤范围内的 `GameFsm` 相关实现，并清理历史构建残留。
## Step 2：核心模型重写要点
### 1) Tile
- 职责保持单一：只管理“一个格子”的占用状态。
- 接口采用教学友好的命名：
  - `occupied()`
  - `occupantId()`
  - `place(unitId)`
  - `clear()`
- 增加显式拷贝构造函数，便于课堂解释对象复制行为。
### 2) Board
- 统一管理两类空间：
  - 棋盘 `tiles_`
  - 备战区 `benchUnits_`
- 为避免 C++17 `std::optional`，查询接口改为返回 `int`：
  - 有单位返回单位 ID
  - 空位返回 `-1`
- 关键接口：
  - `placeOnBoard / clearOnBoard / occupantOnBoard`
  - `placeOnBench / clearOnBench / occupantOnBench`
- 容器访问使用 `.at()`，符合规则中的安全访问建议。
  

### 3) Unit
- `Unit` 作为基类，封装公共属性：`hp/maxHp/attack/attackRange/mana/maxMana`
- 保留课程可解释的继承结构：
  - `WarriorUnit`
  - `MageUnit`
- 行为接口简化为：
  - `takeDamage(amount)`
  - `gainMana(amount)`
  - `isAlive()`
- 维持边界保护：
  - 血量不小于 0
  - 法力不超过 `maxMana`
## Step 3：测试如何对齐重构
### test_board.cpp
- 覆盖初始化、合法放置、非法边界、重复占位和构造异常。
- 所有断言改为匹配新接口命名与 `-1` 哨兵返回约定。
### test_unit.cpp
- 覆盖默认属性、扣血死亡、法力上限、非法构造与死亡后状态不变。
- 接口断言统一为 `maxHp/attackRange/maxMana/takeDamage/gainMana/isAlive`。


## Step 4：当前重构边界
本轮只重建阶段一基础模型，不引入以下后续内容：
- FSM 状态机
- 战斗引擎
- 寻路
- GUI 拖拽
这些功能会在后续步骤按 `development-plan.md` 的顺序推进，避免再次偏离计划。
## Step 5：审查反馈处理说明
### 已处理
1. **析构函数写法**
   - `Unit` 与两个子类改为 `= default`，避免空函数体。
2. **签名一致性**
   - `Board::inBounds` 的头文件声明和实现定义已完全一致。
3. **构造校验意图**
   - 在 `Unit` 构造函数中补充注释，明确 `attack == 0` 是有意保留的设计。

### 有意保留
- `Tile/Board/Unit` 显式拷贝构造函数未改为 `= default`，原因是课程规则明确要求“显式定义拷贝构造函数”，当前实现遵循该教学约束。
## Step 6：Phase 1 第4/5步实现解析
### 1) 拖拽交互层（Step 4）
新增 `DragDropHandler`，把拖拽规则从 UI 解耦成纯逻辑层：
- 输入：`DragLocation`（棋盘坐标或备战区索引）
- 输出：`DragResult`
  - `kSuccess`
  - `kSwapped`
  - `kInvalidSource`
  - `kOutOfBounds`
  - `kSameLocation`
核心流程：
1. 判断是否原地拖拽（`kSameLocation`）
2. 判断目标是否合法（越界则 `kOutOfBounds`）
3. 读取起点单位（为空则 `kInvalidSource`，相当于回弹）
4. 若目标空位则直接移动（`kSuccess`）
5. 若目标有单位则做交换（`kSwapped`）

这样后续接入真实鼠标事件时，只需做坐标映射，不需要改业务规则。
### 2) 基础 GUI 呈现层（Step 5）
新增 `ConsoleRenderer` 作为“可编译、可测试”的基础 GUI 层：
- 渲染棋盘占位（按格子输出）
- 渲染备战区槽位
- 渲染单位属性面板（`HP/Mana/ATK/Range`）
因为当前环境不强依赖 Qt/SDL2，此方案能保证阶段一 GUI 展示目标先落地，且保持后续可平滑替换到图形界面。
### 3) 测试覆盖
`test_drag_drop.cpp` 覆盖了关键交互场景：
- 备战区 -> 棋盘
- 棋盘 -> 备战区
- 棋盘内移动
- 交换
- 空起点拖拽
- 越界拖拽
- 原地拖拽
`test_console_renderer.cpp` 验证输出包含：
- 棋盘标题
- 备战区标题
- 单位占位符
- 属性面板关键字段


最终 21/21 测试通过，说明第4和第5步在当前阶段目标内闭环完成。
## Step 7：需求对齐修复说明
### 1) 玩家实体补齐
新增 `Player` 类作为全局状态载体，当前可支撑：
- `gold`（金币，归属玩家）
- `hp`（血量）
- `level`（等级）
- `populationCap`（人口上限）
- `unitIds`（玩家已有单位）
并提供 `populationOnBoard(const Board&)` 用于统计当前上阵人口。
### 2) 半场规则落地
在 `Board` 中增加：
- `isPlayerHalf(position)`：下半区（`row >= rows/2`）
- `isEnemyHalf(position)`：上半区（`row < rows/2`）
这让“备战区与玩家半场之间拖拽”的规则可以在逻辑层执行。
### 3) 拖拽约束增强
`DragDropHandler` 新增带 `Player` 的构造模式。启用后会做三类检查：
1. 目标棋盘格必须在玩家半场
2. 备战区上阵时不能突破人口上限
3. 拖拽源单位必须属于当前玩家
对应返回值新增：
- `kNotPlayerHalf`
- `kPopulationFull`
### 4) 为什么这一步仍未完全达标
虽然核心规则已经补齐，但仍有一个 requirement 缺口：
- 当前 GUI 仍是控制台渲染，不是 Qt/SDL 图形窗口。
因此“鼠标拖拽 + 图形 GUI”这一要求尚未完全满足，需要下一步接入真实图形框架。
## Step 8：Qt GUI 接入设计说明
### 1) 为什么先做 `SceneCoordMapper`
图形拖拽的核心不是“画图”，而是“像素坐标和逻辑格子坐标的一致转换”。  
因此先把映射逻辑抽到 `SceneCoordMapper`，并保持其不依赖 Qt，这样可以直接用 GoogleTest 做单元验证。
已覆盖场景包括：
- 棋盘中心点映射
- 备战区中心点映射
- 棋盘与备战区间隙（应判定为无效）
- 位置到像素再映射回位置（round-trip）

### 2) 为什么 Qt 构建要做“可选”
当前项目核心是 `core` 逻辑和测试；GUI 是增量层。  
如果强制依赖 Qt，会导致未安装 Qt 的环境无法通过核心构建和测试。
因此在 CMake 采用：
- `option(BUILD_QT_GUI ON)`
- `find_package(Qt6 ... QUIET)`
- 未找到 Qt6 时跳过 GUI target，不影响 `my_auto_arena` 与 `core_tests`
这符合课程开发过程中的“功能增量不破坏主线”原则。
### 3) Qt 图形层如何复用现有业务逻辑
新增 `ArenaScene` 不重新实现规则，而是直接调用已有：
- `DragDropHandler::execute(from, to)`
这样拖拽规则始终与控制台/测试逻辑一致，避免出现“GUI 与核心逻辑分叉”。
执行结果处理分两类：
- 成功：从 `Board` 重同步单位位置，吸附到格子中心
- 失败：播放回弹动画，保持模型状态不变

### 4) 结构拆分
- `TileGraphicsItem`：格子视觉（棋盘半场/备战区）
- `UnitGraphicsItem`：单位卡片视觉 + 鼠标事件
- `ArenaScene`：图元容器与拖拽结果处理
- `UnitInfoPanel`：单位属性展示
- `QtMainWindow`：拼装视图和状态栏反馈
这种拆分便于后续继续扩展到阶段二/三，而不需要重写基础图形骨架。
## Step 9：Phase 2 Step 1（GameFSM）实现解析
### 1) 设计边界
本步只实现“轮次阶段流转控制”，不实现战斗行为本身。  
也就是说，`GameFSM` 不持有 `Board/Unit/Player` 引用，避免把“状态切换”和“战斗执行”耦合在一个类中。
### 2) 状态与转移
当前阶段枚举：
- `kPrepare`
- `kBattle`
- `kSettlement`
转移 API：
- `startBattle()`
- `startSettlement(const RoundOutcome&)`
- `startNextRound()`


每个转移都按统一守卫顺序处理：
1. `gameOver` 优先阻断
2. 重复进入同阶段返回 `kAlreadyInPhase`
3. 非法边返回 `kIllegalTransition`
4. 合法时执行状态更新并返回 `kSuccess`
### 3) 门控与结算数据
- `canPlayerAct()`：仅在 `Prepare` 且未 GameOver 返回 `true`，为后续“战斗阶段禁止拖拽/购买”提供统一判定入口。
- `RoundOutcome`：保存最近一次结算结果（胜负、金币、扣血）。
- `hasOutcome()`：避免在首轮战斗前误读默认结果。
### 4) 测试覆盖策略
`test_game_fsm.cpp` 覆盖了四类关键行为：
- 初始态与完整合法循环
- 非法转移与状态不变断言
- GameOver 阻断
- 拷贝构造与多轮回归（round 单调递增）
该策略能在不引入战斗引擎的前提下，提前锁定 Phase2 流程骨架的正确性。

## Step 10：Phase 2 Step 2（BattleEngine）实现解析
### 1) 设计边界
本步仅实现“战斗 Tick 驱动骨架”，不包含寻路和技能。
- 已实现：索敌、射程判定、普攻伤害、回蓝、死亡清理、胜负结算
- 暂未实现：移动、阻挡绕行、技能释放（后续 Step 3/4）
### 2) Tick 流程
`BattleEngine::tick()` 每次执行：
1. 遍历所有存活单位
2. 选择最近敌方目标（并列按规则打破）
3. 若目标在攻击距离内则造成伤害并回蓝
4. tick 末统一清理死亡单位
5. 检查战斗结束或超时
这种结构将“状态更新”和“清理”分离，降低中途删除对象导致迭代失效的风险。
### 3) 索敌并列规则
当前并列规则（同距离）按以下顺序：
1. 生命值优先
2. 列从左到右
3. 行从下到上

规则在 `selectTarget` 中集中实现，便于后续独立调整策略。
### 4) 敌方生成最小对接
`EnemySpawner` 将设计文档中的前 1~3 轮敌方配置以静态结构落地：
- 模板池（名称/基础属性）
- 按轮次的 `spawnList`
- 敌方半场坐标部署
这使“生成敌人 -> 开战 -> 结算”链路可以在当前阶段直接测试。
### 5) 已做的安全修正
- 禁用 `BattleEngine` 拷贝构造，避免对引用成员的误拷贝。
- 清理死亡单位时同步从战斗注册表中移除，减少后续阶段状态污染。

## Step 11：Phase 2 Step 3（Pathfinder BFS）实现解析

### 1) 为什么选 BFS 而非 A*
课程阶段要求"至少 BFS"，BFS 在无权图中保证最短步数，且实现简单、可解释性强。  
A* 可在后续迭代中通过替换队列为优先队列（`std::priority_queue`）来升级，现有接口无需修改。

### 2) 寻路目标定义
本寻路的目标不是"到达目标单位所在格"（因为那格已被目标占用），而是找到一个满足以下条件的空格：
- 与目标位置的欧氏距离 ≤ `attackRange`（即站在此格可发动攻击）
- 当前为空（可移入）

这种目标定义更接近真实战场行为，也避免了寻路和攻击判定逻辑的重复。

### 3) BFS 路径回溯
BFS 扩展时记录 `parentRow / parentCol` 二维数组（取代链表，便于随机访问）。  
找到目标格后，沿 parent 链反向追踪直到起点，然后用 `std::reverse` 翻转为正向路径。  
只取路径第二个节点（索引 1）作为"下一步"，从而实现"每帧移动一格"。

### 4) 单位自身格的处理
BFS 扩展时，若当前格就是移动单位自身的起始格，则视为可穿越（因为单位"正在离开"）。  
若不做此处理，BFS 在起点判定时会因"自身占用"而提前终止，导致路径不可达。

### 5) 接入 BattleEngine
`BattleEngine` 在每 Tick 的攻击判定中：
1. 先检查目标是否在攻击范围内 → 是则攻击
2. 否则调用 `Pathfinder::nextStepTowardAttackRange` 获取下一步坐标
3. 成功时：更新 Board（清起始格、占目标格），不执行攻击
4. 失败时：单位原地停留（不会产生死循环或崩溃）

## Step 12：Phase 2 Step 4（HeroUnits，技能多态）实现解析

### 1) 多态接口设计
`Unit` 基类中 `castFullManaSkill` 被声明为 `virtual`，默认实现仅清空法力（`spendAllMana()`）：
```cpp
virtual void castFullManaSkill(Board&, std::map<int, Unit*>&, Unit* primaryTarget);
```
各英雄子类通过 `override` 各自实现技能逻辑，体现多态的核心考察点。

### 2) 五种英雄与技能分类
| 英雄 | 技能类型 | 技能效果 |
|------|---------|---------|
| `AshRaiderHero`（灰烬掠袭者） | 单目标爆发 | 对主目标造成 180 点伤害 |
| `NightArcherHero`（夜羽猎弓手） | 单目标穿透（远程） | 对主目标造成 210 点伤害 |
| `CurseHammerHero`（诅印重锤奴） | AOE 4 邻格 | 对自身周围 4 个相邻格中的敌方各造成 120 点伤害 |
| `MistWitchHero`（瘴雾魔女学徒） | 单目标法术 | 对主目标造成 90 点伤害 |
| `BonePrayerHero`（骨契祷告者） | 治疗 | 为血量最低的友方治疗 150；无友方则自愈 120 |

### 3) AOE 技能的坐标查询
`CurseHammerHero` 的技能需要知道自身在棋盘上的坐标，通过 `board.findUnitOnBoard(id())` 查询：
- 若不在棋盘上（离线/备战区），直接跳过并清空法力，不崩溃。
- 遍历 4 方向邻格，对满足条件（存活 + 敌方阵营）的单位造成伤害。

### 4) 治疗技能的友方检索
`BonePrayerHero` 遍历全局 `units` 注册表，筛选出存活的同阵营非自身单位，选出血量最低者。  
遍历使用传统 `std::map::iterator`，符合 C++11/14 教学风格约束。

### 5) `BattleEngine` 如何触发技能
攻击命中后调用 `gainMana(kManaPerAttack)`；若 `mana() >= maxMana()`，则立即调用 `castFullManaSkill`，并传入当前攻击的主目标作为 `primaryTarget`。  
这样技能与普攻在同一帧结算，不引入额外状态机。

## Step 13：Phase 2 Step 5（PvERoundRunner）实现解析

### 1) 职责划分
`PvERoundRunner` 是一个纯静态工具类，不持有成员状态。  
它把"一轮 PvE 的完整生命周期"封装成单一函数调用，调用方（主程序或测试）无需手动管理生成-战斗-结算-清理的顺序。

### 2) runRoundBattle 的完整流程
```text
EnemySpawner::configForRound(roundIndex)  → 获取本轮配置（奖励/扣血/生成列表）
EnemySpawner::spawnRound(...)             → 生成敌方单位并部署到 Board
BattleEngine engine(board, units)         → 创建战斗引擎
engine.setDefeatHpPenalty(...)            → 设置输了扣玩家多少血
while (!engine.isFinished()) tick()       → 驱动至战斗结束
engine.outcome()                          → 取结算结果
Player::setGold / setHp                   → 更新玩家资源
removeEnemyUnits(board, units)            → 清理敌方单位指针
```

### 3) removeEnemyUnits 的内存安全
遍历 `units` 注册表，逐个找出 `owner == enemy` 的单位：
- 调用 `board.clearOnBoard(pos)` 清空棋盘格占用
- `delete unit` 释放堆内存
- `units.erase(it)` 并安全推进迭代器（避免悬空指针）

这是课程中"RAII / new-delete 配对"考察点的具体体现。

### 4) 与 GameFSM 的配合方式
`PvERoundRunner::runRoundBattle` 返回 `RoundOutcome`，调用方将其传入 `GameFSM::startSettlement(outcome)`，完成阶段切换与结算数据记录。  
两者职责明确分离：FSM 只管阶段流转，Runner 只管战斗驱动与资源结算。



