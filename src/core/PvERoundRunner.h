#ifndef MY_AUTO_ARENA_CORE_PVE_ROUND_RUNNER_H
#define MY_AUTO_ARENA_CORE_PVE_ROUND_RUNNER_H

#include <map>

#include "core/Board.h"
#include "core/EnemySpawner.h"
#include "core/GameFSM.h"
#include "core/Player.h"

namespace my_auto_arena {
namespace core {

class Unit;

// 单轮 PvE：生成敌军、驱动 BattleEngine 至结束，结算金币/扣血，并清理敌方单位指针。
class PvERoundRunner {
public:
    static RoundOutcome runRoundBattle(Board& board, Player& player, std::map<int, Unit*>& units, int roundIndex,
                                       EnemySpawner& spawner, int& nextUnitId);

    static void removeEnemyUnits(Board& board, std::map<int, Unit*>& units);
};

}  // namespace core
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_CORE_PVE_ROUND_RUNNER_H
