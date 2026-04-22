#ifndef MY_AUTO_ARENA_CORE_BATTLE_ENGINE_H
#define MY_AUTO_ARENA_CORE_BATTLE_ENGINE_H

#include <map>

#include "core/Board.h"
#include "core/GameFSM.h"
#include "core/Unit.h"

namespace my_auto_arena {
namespace core {

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

private:
    Board& board_;
    std::map<int, Unit*>& units_;
    int tickCount_;
    bool finished_;
    RoundOutcome outcome_;
    int defeatHpPenalty_;

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
