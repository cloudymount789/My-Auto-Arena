#include "core/EnemySpawner.h"

#include <algorithm>
#include <cmath>

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
    // 敌方基础值略高于对应玩家英雄，配合升星倍率形成后期压力。
    static const std::vector<EnemyTemplate> kTemplates = {
        {"战士",     1700, 68, 1, 75},   // 0: 前排, 追着玩家打
        {"射手",     1300, 65, 4, 75},   // 1: 远程, 优先射低血量目标
        {"重甲战士", 2800, 52, 1, 90},   // 2: 超强肉盾 AOE
        {"法师",     1050, 45, 3, 70},   // 3: 爆发型, 技能快
        {"治疗师",   1450, 30, 3, 80},   // 4: 后排辅助, 敌方持久战关键
        {"攻城弩",   3200, 90, 5, 110},  // 5: BOSS级远程, 第6关核心威胁
    };
    return kTemplates;
}

LevelConfig EnemySpawner::configForRound(int round) const {
    if (round < 1) {
        round = 1;
    }
    LevelConfig cfg;
    cfg.roundIndex = round;
    cfg.onLosePlayerHpDamage = 0;
    cfg.winGoldReward = 0;

    // ─────────────────────────────────────────────────────────────────────
    // 前6关：精心设计的新手曲线，让玩家先赢后感受压力
    // 第7关起：使用无尽模板 + scaleStatForRound 指数膨胀
    // ─────────────────────────────────────────────────────────────────────

    // 第1关：2个战士，玩家出发仅有2个英雄，数量平等但统计偏强。
    if (round == 1) {
        cfg.spawnList.push_back(SpawnEntry{0, 1, Position{1, 2}});
        cfg.spawnList.push_back(SpawnEntry{0, 1, Position{1, 5}});
        cfg.onLosePlayerHpDamage = 5;
        cfg.winGoldReward = 4;
        return cfg;
    }
    // 第2关：3个混合，射手+战士+战士，玩家应购入第3英雄。
    if (round == 2) {
        cfg.spawnList.push_back(SpawnEntry{0, 1, Position{1, 2}});
        cfg.spawnList.push_back(SpawnEntry{1, 1, Position{0, 4}});
        cfg.spawnList.push_back(SpawnEntry{0, 1, Position{1, 6}});
        cfg.onLosePlayerHpDamage = 8;
        cfg.winGoldReward = 5;
        return cfg;
    }
    // 第3关：4个，重甲战士+射手+战士+法师，首次出现法师。
    if (round == 3) {
        cfg.spawnList.push_back(SpawnEntry{2, 1, Position{1, 1}});
        cfg.spawnList.push_back(SpawnEntry{1, 1, Position{0, 3}});
        cfg.spawnList.push_back(SpawnEntry{0, 1, Position{1, 5}});
        cfg.spawnList.push_back(SpawnEntry{3, 1, Position{0, 6}});
        cfg.onLosePlayerHpDamage = 12;
        cfg.winGoldReward = 6;
        return cfg;
    }
    // 第4关：5个，★2重甲战士领队，射手×2+法师+治疗师，出现治疗师使敌方持久化。
    if (round == 4) {
        cfg.spawnList.push_back(SpawnEntry{2, 2, Position{1, 2}});
        cfg.spawnList.push_back(SpawnEntry{1, 1, Position{0, 4}});
        cfg.spawnList.push_back(SpawnEntry{1, 1, Position{0, 6}});
        cfg.spawnList.push_back(SpawnEntry{3, 1, Position{0, 2}});
        cfg.spawnList.push_back(SpawnEntry{4, 1, Position{0, 0}});
        cfg.onLosePlayerHpDamage = 15;
        cfg.winGoldReward = 8;
        return cfg;
    }
    // 第5关：5个，★2重甲战士+★2射手+法师×2+治疗师，远程输出最强一关。
    if (round == 5) {
        cfg.spawnList.push_back(SpawnEntry{2, 2, Position{1, 1}});
        cfg.spawnList.push_back(SpawnEntry{1, 2, Position{0, 3}});
        cfg.spawnList.push_back(SpawnEntry{3, 1, Position{0, 5}});
        cfg.spawnList.push_back(SpawnEntry{3, 1, Position{0, 6}});
        cfg.spawnList.push_back(SpawnEntry{4, 1, Position{0, 1}});
        cfg.onLosePlayerHpDamage = 18;
        cfg.winGoldReward = 9;
        return cfg;
    }
    // 第6关：BOSS局，攻城弩★2领队，重甲战士+战士×2+法师+治疗师。
    if (round == 6) {
        cfg.spawnList.push_back(SpawnEntry{5, 2, Position{0, 3}});
        cfg.spawnList.push_back(SpawnEntry{2, 1, Position{1, 2}});
        cfg.spawnList.push_back(SpawnEntry{0, 1, Position{1, 1}});
        cfg.spawnList.push_back(SpawnEntry{0, 1, Position{1, 5}});
        cfg.spawnList.push_back(SpawnEntry{3, 1, Position{0, 5}});
        cfg.spawnList.push_back(SpawnEntry{4, 1, Position{0, 1}});
        cfg.onLosePlayerHpDamage = 25;
        cfg.winGoldReward = 12;
        return cfg;
    }

    // ─────────────────────────────────────────────────────────────────────
    // 第7关起：无尽模式，使用6关的模板结构，对 HP/ATK 按指数公式膨胀。
    // 每3关循环一组阵型（轻量/标准/精英），并且关数越高组内全部升星。
    // ─────────────────────────────────────────────────────────────────────
    // 动态 star level: round 7-9 -> star1, round 10-12 -> star2, round 13+ -> star2
    const int baseStar = (round >= 13) ? 2 : (round >= 10 ? 2 : 1);
    const int eliteStar = (round >= 10) ? 2 : 1;

    // 每关 12% 指数增长；statScaleFactor 由 spawnRound 读取并应用到模板 hp/atk。
    const double factor = std::pow(1.12, round - 6);
    cfg.statScaleFactor = factor;

    // 阵型轮换（每3关一个周期）
    const int cycle = ((round - 7) % 3);
    if (cycle == 0) {
        // 轻量阵：战士×2+射手×2+法师
        cfg.spawnList.push_back(SpawnEntry{0, baseStar, Position{1, 2}});
        cfg.spawnList.push_back(SpawnEntry{0, baseStar, Position{1, 5}});
        cfg.spawnList.push_back(SpawnEntry{1, baseStar, Position{0, 3}});
        cfg.spawnList.push_back(SpawnEntry{1, baseStar, Position{0, 5}});
        cfg.spawnList.push_back(SpawnEntry{3, baseStar, Position{0, 1}});
    } else if (cycle == 1) {
        // 标准阵：重甲战士+战士+射手+法师×2+治疗师
        cfg.spawnList.push_back(SpawnEntry{2, eliteStar, Position{1, 2}});
        cfg.spawnList.push_back(SpawnEntry{0, baseStar,  Position{1, 4}});
        cfg.spawnList.push_back(SpawnEntry{1, eliteStar, Position{0, 4}});
        cfg.spawnList.push_back(SpawnEntry{3, baseStar,  Position{0, 2}});
        cfg.spawnList.push_back(SpawnEntry{3, baseStar,  Position{0, 6}});
        cfg.spawnList.push_back(SpawnEntry{4, baseStar,  Position{0, 0}});
    } else {
        // 精英阵：攻城弩+重甲战士+战士×2+法师+治疗师
        cfg.spawnList.push_back(SpawnEntry{5, eliteStar, Position{0, 3}});
        cfg.spawnList.push_back(SpawnEntry{2, eliteStar, Position{1, 2}});
        cfg.spawnList.push_back(SpawnEntry{0, baseStar,  Position{1, 1}});
        cfg.spawnList.push_back(SpawnEntry{0, baseStar,  Position{1, 5}});
        cfg.spawnList.push_back(SpawnEntry{3, baseStar,  Position{0, 5}});
        cfg.spawnList.push_back(SpawnEntry{4, baseStar,  Position{0, 1}});
    }

    // 失败惩罚和金币奖励随关数增长（上限分别为50和30）
    cfg.onLosePlayerHpDamage = std::min(50, 25 + (round - 6) * 3);
    cfg.winGoldReward        = std::min(30, 12 + (round - 6) * 2);
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
        // 无尽关卡：对模板 hp/atk 应用 statScaleFactor，再交给 SpawnedEnemyUnit 做升星乘算。
        EnemyTemplate scaledTpl = tpl.at(entry.templateIndex);
        if (cfg.statScaleFactor > 1.0 + 1e-6) {
            scaledTpl.hp  = static_cast<int>(scaledTpl.hp  * cfg.statScaleFactor + 0.5);
            scaledTpl.atk = static_cast<int>(scaledTpl.atk * cfg.statScaleFactor + 0.5);
        }
        Unit* unit = new SpawnedEnemyUnit(nextUnitId, scaledTpl, entry.starLevel);
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
