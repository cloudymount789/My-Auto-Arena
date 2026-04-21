#ifndef MY_AUTO_ARENA_CORE_ENEMY_SPAWNER_H
#define MY_AUTO_ARENA_CORE_ENEMY_SPAWNER_H

#include <string>
#include <vector>

#include "core/Board.h"
#include "core/Unit.h"

namespace my_auto_arena {
namespace core {

struct EnemyTemplate {
    std::string name;
    int hp;
    int atk;
    int range;
    int maxMana;
};

struct SpawnEntry {
    int templateIndex;
    int starLevel;
    Position deployPos;
};

struct LevelConfig {
    int roundIndex;
    std::vector<SpawnEntry> spawnList;
    int onLosePlayerHpDamage;
    int winGoldReward;
};

class EnemySpawner {
public:
    EnemySpawner();
    EnemySpawner(const EnemySpawner& other);

    const std::vector<EnemyTemplate>& templates() const;
    LevelConfig configForRound(int round) const;
    std::vector<Unit*> spawnRound(int round, Board& board, int& nextUnitId) const;
};

}  // namespace core
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_CORE_ENEMY_SPAWNER_H
