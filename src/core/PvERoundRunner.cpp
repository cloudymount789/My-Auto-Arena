#include "core/PvERoundRunner.h"

#include "core/BattleEngine.h"
#include "core/Unit.h"

namespace my_auto_arena {
namespace core {

RoundOutcome PvERoundRunner::runRoundBattle(Board& board, Player& player, std::map<int, Unit*>& units, int roundIndex,
                                            EnemySpawner& spawner, int& nextUnitId) {
    const LevelConfig cfg = spawner.configForRound(roundIndex);
    std::vector<Unit*> spawned = spawner.spawnRound(roundIndex, board, nextUnitId);
    for (std::size_t i = 0; i < spawned.size(); ++i) {
        Unit* unit = spawned.at(i);
        units[unit->id()] = unit;
    }

    BattleEngine engine(board, units);
    engine.setDefeatHpPenalty(cfg.onLosePlayerHpDamage);
    while (!engine.isFinished()) {
        engine.tick();
    }

    RoundOutcome outcome = engine.outcome();
    if (outcome.playerWon) {
        outcome.goldReward = cfg.winGoldReward;
        player.setGold(player.gold() + outcome.goldReward);
    } else {
        outcome.hpPenalty = cfg.onLosePlayerHpDamage;
        const int nextHp = player.hp() - outcome.hpPenalty;
        player.setHp(nextHp < 0 ? 0 : nextHp);
    }

    removeEnemyUnits(board, units);
    return outcome;
}

void PvERoundRunner::removeEnemyUnits(Board& board, std::map<int, Unit*>& units) {
    for (std::map<int, Unit*>::iterator it = units.begin(); it != units.end();) {
        Unit* unit = it->second;
        if (unit == nullptr || unit->owner() != UnitOwner::enemy) {
            ++it;
            continue;
        }
        const Position pos = board.findUnitOnBoard(unit->id());
        if (board.inBounds(pos)) {
            board.clearOnBoard(pos);
        }
        delete unit;
        it = units.erase(it);
    }
}

}  // namespace core
}  // namespace my_auto_arena
