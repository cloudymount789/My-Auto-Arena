#include "core/EnemySpawner.h"

#include <stdexcept>

namespace my_auto_arena {
namespace core {

namespace {

int scaleStat(int base, int starLevel) {
    if (starLevel >= 2) {
        return static_cast<int>(base * 1.5 + 0.5);
    }
    return base;
}

class SpawnedEnemyUnit final : public Unit {
public:
    SpawnedEnemyUnit(int id, const EnemyTemplate& tpl, int starLevel)
        : Unit(id, tpl.name, UnitOwner::enemy, scaleStat(tpl.hp, starLevel), scaleStat(tpl.atk, starLevel), tpl.range,
               tpl.maxMana) {}
    // 显式拷贝构造函数：委托给 Unit，符合课程对所有实体类显式定义拷贝构造的要求。
    SpawnedEnemyUnit(const SpawnedEnemyUnit& other) : Unit(other) {}
    virtual ~SpawnedEnemyUnit() override = default;
};
}  // namespace

EnemySpawner::EnemySpawner() {}

const std::vector<EnemyTemplate>& EnemySpawner::templates() const {
    // 模板顺序：战士 / 射手 / 重甲战士 / 法师 / 治疗师 / 攻城弩
    // HP 与玩家英雄同量级（2000-3200），确保战斗持续 15-30 Tick 以上。
    static const std::vector<EnemyTemplate> kTemplates = {
        {"战士",     2200, 62, 1, 80},
        {"射手",     1800, 58, 4, 90},
        {"重甲战士", 3200, 50, 1, 100},
        {"法师",     1500, 38, 3, 90},
        {"治疗师",   1800, 30, 3, 100},
        {"攻城弩",   2500, 70, 5, 110},
    };
    return kTemplates;
}

LevelConfig EnemySpawner::configForRound(int round) const {
    if (round < 1 || round > 6) {
        throw std::out_of_range("EnemySpawner: round index out of range [1..6].");
    }
    LevelConfig cfg;
    cfg.roundIndex = round;
    cfg.onLosePlayerHpDamage = 0;
    cfg.winGoldReward = 0;
    if (round == 1) {
        cfg.spawnList.push_back(SpawnEntry{0, 1, Position{1, 3}});
        cfg.onLosePlayerHpDamage = 2;
        cfg.winGoldReward = 5;
        return cfg;
    }
    if (round == 2) {
        cfg.spawnList.push_back(SpawnEntry{0, 1, Position{1, 2}});
        cfg.spawnList.push_back(SpawnEntry{1, 1, Position{0, 4}});
        cfg.onLosePlayerHpDamage = 3;
        cfg.winGoldReward = 6;
        return cfg;
    }
    if (round == 3) {
        cfg.spawnList.push_back(SpawnEntry{2, 1, Position{1, 3}});
        cfg.spawnList.push_back(SpawnEntry{1, 1, Position{0, 4}});
        cfg.onLosePlayerHpDamage = 4;
        cfg.winGoldReward = 7;
        return cfg;
    }
    if (round == 4) {
        cfg.spawnList.push_back(SpawnEntry{2, 1, Position{2, 2}});
        cfg.spawnList.push_back(SpawnEntry{3, 1, Position{1, 4}});
        cfg.spawnList.push_back(SpawnEntry{1, 1, Position{0, 5}});
        cfg.onLosePlayerHpDamage = 5;
        cfg.winGoldReward = 8;
        return cfg;
    }
    if (round == 5) {
        cfg.spawnList.push_back(SpawnEntry{2, 2, Position{2, 3}});
        cfg.spawnList.push_back(SpawnEntry{3, 1, Position{1, 5}});
        cfg.spawnList.push_back(SpawnEntry{4, 1, Position{0, 2}});
        cfg.onLosePlayerHpDamage = 6;
        cfg.winGoldReward = 9;
        return cfg;
    }
    if (round == 6) {
        cfg.spawnList.push_back(SpawnEntry{5, 1, Position{0, 3}});
        cfg.spawnList.push_back(SpawnEntry{2, 1, Position{2, 4}});
        cfg.spawnList.push_back(SpawnEntry{0, 1, Position{1, 1}});
        cfg.spawnList.push_back(SpawnEntry{1, 1, Position{0, 6}});
        cfg.onLosePlayerHpDamage = 7;
        cfg.winGoldReward = 10;
        return cfg;
    }
    return cfg;
}

std::vector<Unit*> EnemySpawner::spawnRound(int round, Board& board, int& nextUnitId) const {
    LevelConfig cfg = configForRound(round);
    std::vector<Unit*> spawned;
    const std::vector<EnemyTemplate>& tpl = templates();
    for (std::size_t i = 0; i < cfg.spawnList.size(); ++i) {
        const SpawnEntry& entry = cfg.spawnList.at(i);
        if (entry.templateIndex < 0 || entry.templateIndex >= static_cast<int>(tpl.size()) ||
            !board.inBounds(entry.deployPos) || !board.isEnemyHalf(entry.deployPos)) {
            continue;
        }
        Unit* unit = new SpawnedEnemyUnit(nextUnitId, tpl.at(entry.templateIndex), entry.starLevel);
        if (!board.placeOnBoard(unit->id(), entry.deployPos)) {
            delete unit;
            continue;
        }
        spawned.push_back(unit);
        ++nextUnitId;
    }
    return spawned;
}

}  // namespace core
}  // namespace my_auto_arena
