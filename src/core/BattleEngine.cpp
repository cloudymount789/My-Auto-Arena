#include "core/BattleEngine.h"

#include <climits>

#include "core/Pathfinder.h"

namespace my_auto_arena {
namespace core {

BattleEngine::BattleEngine(Board& board, std::map<int, Unit*>& units)
    : board_(board), units_(units), tickCount_(0), finished_(false), outcome_{false, 0, 0}, defeatHpPenalty_(4) {}

void BattleEngine::setDefeatHpPenalty(int hpPenalty) { defeatHpPenalty_ = hpPenalty; }

void BattleEngine::tick() {
    if (finished_) {
        return;
    }

    ++tickCount_;

    // Phase 1 — strikes and full-mana skills only. Resolving movement in the same pass as
    // attacks lets a lower map id move into range before the opponent acts, giving a free
    // first hit on the closing unit; splitting phases keeps contact exchanges symmetric.
    for (std::map<int, Unit*>::iterator it = units_.begin(); it != units_.end(); ++it) {
        Unit* attacker = it->second;
        if (attacker == nullptr || !attacker->isAlive()) {
            continue;
        }
        Unit* target = selectTarget(*attacker);
        if (target == nullptr) {
            continue;
        }
        const Position attackerPos = board_.findUnitOnBoard(attacker->id());
        const Position targetPos = board_.findUnitOnBoard(target->id());

        if (attacker->mana() >= attacker->maxMana()) {
            attacker->castFullManaSkill(board_, units_, target);
            continue;
        }

        if (inRange(*attacker, attackerPos, targetPos)) {
            target->takeDamage(attacker->attack());
            attacker->gainMana(kManaPerAttack);
        }
    }

    clearDeadUnits();
    resolveEndState();
    if (finished_) {
        return;
    }

    // Phase 2 — movement only for units still out of attack range (full-mana casts already ran in phase 1).
    for (std::map<int, Unit*>::iterator it = units_.begin(); it != units_.end(); ++it) {
        Unit* attacker = it->second;
        if (attacker == nullptr || !attacker->isAlive()) {
            continue;
        }
        Unit* target = selectTarget(*attacker);
        if (target == nullptr) {
            continue;
        }
        const Position attackerPos = board_.findUnitOnBoard(attacker->id());
        const Position targetPos = board_.findUnitOnBoard(target->id());
        if (inRange(*attacker, attackerPos, targetPos)) {
            continue;
        }
        tryMoveTowardTarget(attacker, target);
    }

    clearDeadUnits();
    resolveEndState();

    if (!finished_ && tickCount_ >= kMaxTicks) {
        finished_ = true;
        outcome_.playerWon = false;
        outcome_.goldReward = 1;
        outcome_.hpPenalty = defeatHpPenalty_;
    }
}

bool BattleEngine::isFinished() const { return finished_; }

int BattleEngine::tickCount() const { return tickCount_; }

RoundOutcome BattleEngine::outcome() const { return outcome_; }

Unit* BattleEngine::selectTarget(const Unit& attacker) const {
    const Position attackerPos = board_.findUnitOnBoard(attacker.id());
    if (!board_.inBounds(attackerPos)) {
        return nullptr;
    }

    Unit* best = nullptr;
    int bestDist = INT_MAX;
    for (std::map<int, Unit*>::const_iterator cit = units_.begin(); cit != units_.end(); ++cit) {
        Unit* candidate = cit->second;
        if (candidate == nullptr || !candidate->isAlive() || candidate->owner() == attacker.owner()) {
            continue;
        }
        const Position candidatePos = board_.findUnitOnBoard(candidate->id());
        if (!board_.inBounds(candidatePos)) {
            continue;
        }
        const int d = distanceSquared(attackerPos, candidatePos);
        if (best == nullptr || d < bestDist) {
            best = candidate;
            bestDist = d;
            continue;
        }
        if (d == bestDist) {
            Position bestPos = board_.findUnitOnBoard(best->id());
            // Prefer finishing low-HP targets; break ties deterministically by grid position.
            if (candidate->hp() < best->hp() || (candidate->hp() == best->hp() && candidatePos.col < bestPos.col) ||
                (candidate->hp() == best->hp() && candidatePos.col == bestPos.col && candidatePos.row > bestPos.row)) {
                best = candidate;
            }
        }
    }
    return best;
}

bool BattleEngine::inRange(const Unit& attacker, Position attackerPos, Position targetPos) const {
    if (!board_.inBounds(attackerPos) || !board_.inBounds(targetPos)) {
        return false;
    }
    const int rangeSq = attacker.attackRange() * attacker.attackRange();
    return distanceSquared(attackerPos, targetPos) <= rangeSq;
}

int BattleEngine::distanceSquared(Position a, Position b) const {
    const int dr = a.row - b.row;
    const int dc = a.col - b.col;
    return dr * dr + dc * dc;
}

void BattleEngine::tryMoveTowardTarget(Unit* attacker, Unit* target) {
    const Position start = board_.findUnitOnBoard(attacker->id());
    const Position targetPos = board_.findUnitOnBoard(target->id());
    if (!board_.inBounds(start) || !board_.inBounds(targetPos)) {
        return;
    }
    Position next;
    if (!Pathfinder::nextStepTowardAttackRange(board_, units_, attacker->id(), start, targetPos, attacker->attackRange(),
                                               &next)) {
        return;
    }
    if (board_.occupantOnBoard(next) != Board::kEmptySlot) {
        return;
    }
    board_.clearOnBoard(start);
    if (!board_.placeOnBoard(attacker->id(), next)) {
        (void)board_.placeOnBoard(attacker->id(), start);
    }
}

void BattleEngine::clearDeadUnits() {
    for (std::map<int, Unit*>::iterator it = units_.begin(); it != units_.end();) {
        Unit* unit = it->second;
        if (unit == nullptr || unit->isAlive()) {
            ++it;
            continue;
        }
        const Position pos = board_.findUnitOnBoard(unit->id());
        if (board_.inBounds(pos)) {
            board_.clearOnBoard(pos);
        }
        it = units_.erase(it);
    }
}

bool BattleEngine::allDead(UnitOwner owner) const {
    for (std::map<int, Unit*>::const_iterator it = units_.begin(); it != units_.end(); ++it) {
        Unit* unit = it->second;
        if (unit != nullptr && unit->owner() == owner && unit->isAlive()) {
            return false;
        }
    }
    return true;
}

void BattleEngine::resolveEndState() {
    const bool playerDead = allDead(UnitOwner::player);
    const bool enemyDead = allDead(UnitOwner::enemy);
    if (!playerDead && !enemyDead) {
        return;
    }
    finished_ = true;
    if (enemyDead && !playerDead) {
        outcome_.playerWon = true;
        outcome_.goldReward = 3;
        outcome_.hpPenalty = 0;
    } else {
        outcome_.playerWon = false;
        outcome_.goldReward = 1;
        outcome_.hpPenalty = defeatHpPenalty_;
    }
}

}  // namespace core
}  // namespace my_auto_arena
