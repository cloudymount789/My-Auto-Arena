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
    int physDef;  // 基础物理防御；高物防单位适合用法师对抗
    int magDef;   // 基础法术防御
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
    double statScaleFactor = 1.0;  // 无尽关卡额外乘以敌方 HP/ATK 的系数
};

class EnemySpawner {
public:
    EnemySpawner();
    // EnemySpawner 无数据成员，显式声明拷贝构造函数符合课程规范；实现委托给默认行为。
    EnemySpawner(const EnemySpawner& other) = default;

    const std::vector<EnemyTemplate>& templates() const;
    LevelConfig configForRound(int round) const;
    std::vector<Unit*> spawnRound(int round, Board& board, int& nextUnitId) const;
};

}  // namespace core
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_CORE_ENEMY_SPAWNER_H
