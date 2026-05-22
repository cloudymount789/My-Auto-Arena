#ifndef MY_AUTO_ARENA_CORE_BATTLE_ENGINE_H
#define MY_AUTO_ARENA_CORE_BATTLE_ENGINE_H

#include <map>
#include <vector>

#include "core/Board.h"
#include "core/GameFSM.h"
#include "core/Unit.h"

namespace my_auto_arena {
namespace core {

// 每 tick 记录的战斗事件，用于驱动 GUI 特效动画。
struct BattleEvent {
    enum class Type { kAttack, kSkill };
    Type type;
    int sourceId;            // 攻击者/施法者单位 ID
    int targetId;            // 目标单位 ID（-1 表示无具体目标）
    bool isMelee;            // 攻击事件：true=近战, false=远程
    UnitClass sourceClass;   // 施法者/攻击者职业（驱动特效颜色区分）
    UnitOwner sourceOwner;   // 攻击者阵营（玩家/敌方，用于区分特效风格）
    int srcRow, srcCol;      // 事件发生时施法者的棋盘坐标（-1 表示不在棋盘）
    int tgtRow, tgtCol;      // 事件发生时目标的棋盘坐标
};

class BattleEngine {
public:
    static const int kMaxTicks = 5000;
    static const int kManaPerAttack = 10;

    BattleEngine(Board& board, std::map<int, Unit*>& units);
    BattleEngine(const BattleEngine& other) = delete;
    BattleEngine& operator=(const BattleEngine& other) = delete;

    void tick();
    bool isFinished() const;
    int tickCount() const;
    RoundOutcome outcome() const;
    void setDefeatHpPenalty(int hpPenalty);

    // 返回最近一次 tick 中产生的战斗事件列表，供 GUI 渲染特效。
    const std::vector<BattleEvent>& lastTickEvents() const;

private:
    Board& board_;
    std::map<int, Unit*>& units_;
    int tickCount_;
    bool finished_;
    RoundOutcome outcome_;
    int defeatHpPenalty_;
    std::vector<BattleEvent> tickEvents_;  // 本 tick 产生的事件，每 tick 开始时清空

    Unit* selectTarget(const Unit& attacker) const;
    bool inRange(const Unit& attacker, Position attackerPos, Position targetPos) const;
    int distanceSquared(Position a, Position b) const;
    void clearDeadUnits();
    bool allDead(UnitOwner owner) const;
    void resolveEndState();
    void tryMoveTowardTarget(Unit* attacker, Unit* target);
};

}  // namespace core
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_CORE_BATTLE_ENGINE_H
