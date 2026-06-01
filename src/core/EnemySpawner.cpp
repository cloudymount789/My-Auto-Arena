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

double starHpMultiplier(int starLevel) {
    return (starLevel >= 2) ? 1.5 : 1.0;
}

double endlessTargetArmyHp(int round) {
    return 9000.0 * std::pow(1.12, round - 7);
}

double plannedArmyHp(const std::vector<SpawnEntry>& entries, const std::vector<EnemyTemplate>& templates) {
    double total = 0.0;
    for (std::size_t i = 0; i < entries.size(); ++i) {
        const SpawnEntry& entry = entries.at(i);
        if (entry.templateIndex < 0 || entry.templateIndex >= static_cast<int>(templates.size())) {
            continue;
        }
        total += templates.at(entry.templateIndex).hp * starHpMultiplier(entry.starLevel);
    }
    return total;
}

class SpawnedEnemyUnit final : public Unit {
public:
    SpawnedEnemyUnit(int id, const EnemyTemplate& tpl, int starLevel)
        : Unit(id, tpl.name, UnitOwner::enemy, scaleStat(tpl.hp, starLevel), scaleStat(tpl.atk, starLevel), tpl.range,
               tpl.maxMana) {
        // 防御值随升星同比缩放，与生命/物攻保持一致的成长规律。
        setBasePhysicalDef(scaleStat(tpl.physDef, starLevel));
        setBaseMagicDef(scaleStat(tpl.magDef, starLevel));
    }
    // 显式拷贝构造函数：委托给 Unit，符合课程对所有实体类显式定义拷贝构造的要求。
    SpawnedEnemyUnit(const SpawnedEnemyUnit& other) : Unit(other) {}
    virtual ~SpawnedEnemyUnit() override = default;
};
}  // namespace

EnemySpawner::EnemySpawner() {}

const std::vector<EnemyTemplate>& EnemySpawner::templates() const {
    // 模板顺序：战士 / 射手 / 重甲战士 / 法师 / 治疗师 / 攻城弩
    // 敌方基础值接近但仍低于对应玩家英雄；难度主要由每关预算曲线控制。
    // 字段顺序：name / hp / atk / range / maxMana / physDef / magDef
    static const std::vector<EnemyTemplate> kTemplates = {
        {"战士",     1300, 54, 1,  75,  18,  6},  // 0: 轻甲前排
        {"射手",      980, 56, 4,  75,   7,  6},  // 1: 皮甲远程
        {"重甲战士", 2200, 44, 1,  90,  55, 18},  // 2: 重甲肉盾
        {"法师",      900, 36, 3,  70,   7, 28},  // 3: 法术单位
        {"治疗师",   1250, 26, 3,  80,   7, 22},  // 4: 法系辅助
        {"攻城弩",   2800, 72, 5, 110,  70, 18},  // 5: BOSS重型机械
    };
    return kTemplates;
}

// 流程：校验回合数 ──> 初始化 LevelConfig ──> 前6关走固定新手曲线 ──> 第7关起计算星级/属性倍率 ──> 按周期轮换阵型 ──> 设置奖惩并返回
LevelConfig EnemySpawner::configForRound(int round) const {
    if (round < 1) {
        round = 1;
    }
    LevelConfig cfg;
    cfg.roundIndex = round;
    cfg.onLosePlayerHpDamage = 0;
    cfg.winGoldReward = 0;

    // ─────────────────────────────────────────────────────────────────────
    // 前6关：新手曲线 —— 前3关教学，4-5关推动玩家买人/升人口，第6关是首个BOSS检验。
    // 第7关起：用目标总生命预算反推倍率，平滑抵消阵型轮换造成的战力波动。
    // ─────────────────────────────────────────────────────────────────────

    // 第1关：2个战士，玩家出发仅有2个英雄，应轻松取胜。
    if (round == 1) {
        cfg.statScaleFactor = 0.53;
        cfg.spawnList.push_back(SpawnEntry{0, 1, Position{1, 2}});
        cfg.spawnList.push_back(SpawnEntry{0, 1, Position{1, 5}});
        cfg.onLosePlayerHpDamage = 5;
        cfg.winGoldReward = 4;
        return cfg;
    }
    // 第2关：3个混合，射手+战士+战士。
    if (round == 2) {
        cfg.statScaleFactor = 0.62;
        cfg.spawnList.push_back(SpawnEntry{0, 1, Position{1, 2}});
        cfg.spawnList.push_back(SpawnEntry{1, 1, Position{0, 4}});
        cfg.spawnList.push_back(SpawnEntry{0, 1, Position{1, 6}});
        cfg.onLosePlayerHpDamage = 8;
        cfg.winGoldReward = 5;
        return cfg;
    }
    // 第3关：4个，重甲战士+射手+战士+法师。
    if (round == 3) {
        cfg.statScaleFactor = 0.72;
        cfg.spawnList.push_back(SpawnEntry{2, 1, Position{1, 1}});
        cfg.spawnList.push_back(SpawnEntry{1, 1, Position{0, 3}});
        cfg.spawnList.push_back(SpawnEntry{0, 1, Position{1, 5}});
        cfg.spawnList.push_back(SpawnEntry{3, 1, Position{0, 6}});
        cfg.onLosePlayerHpDamage = 12;
        cfg.winGoldReward = 6;
        return cfg;
    }
    // 第4关：5个，重甲战士领队，射手×2+法师+治疗师（均为1星）。
    if (round == 4) {
        cfg.statScaleFactor = 0.84;
        cfg.spawnList.push_back(SpawnEntry{2, 1, Position{1, 2}});
        cfg.spawnList.push_back(SpawnEntry{1, 1, Position{0, 4}});
        cfg.spawnList.push_back(SpawnEntry{1, 1, Position{0, 6}});
        cfg.spawnList.push_back(SpawnEntry{3, 1, Position{0, 2}});
        cfg.spawnList.push_back(SpawnEntry{4, 1, Position{0, 0}});
        cfg.onLosePlayerHpDamage = 15;
        cfg.winGoldReward = 8;
        return cfg;
    }
    // 第5关：5个，重甲战士+射手×2+法师×2+治疗师（均为1星）。
    if (round == 5) {
        cfg.statScaleFactor = 0.96;
        cfg.spawnList.push_back(SpawnEntry{2, 1, Position{1, 1}});
        cfg.spawnList.push_back(SpawnEntry{1, 1, Position{0, 3}});
        cfg.spawnList.push_back(SpawnEntry{3, 1, Position{0, 5}});
        cfg.spawnList.push_back(SpawnEntry{3, 1, Position{0, 6}});
        cfg.spawnList.push_back(SpawnEntry{4, 1, Position{0, 1}});
        cfg.onLosePlayerHpDamage = 18;
        cfg.winGoldReward = 9;
        return cfg;
    }
    // 第6关：BOSS局，攻城弩★2领队，重甲战士+战士×2+法师+治疗师。
    if (round == 6) {
        cfg.statScaleFactor = 0.98;
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
    // 第7关起：无尽模式。每3关循环一组阵型（轻量/标准/精英），并且关数越高组内全部升星。
    // 倍率由目标总生命预算反推：第7关约9000总生命，之后每关约+12%，让压力递进但不突刺。
    // ─────────────────────────────────────────────────────────────────────
    // 动态星级：第 7–9 关为 1 星，第 10–12 关为 2 星，第 13 关及以上为 2 星
    const int baseStar = (round >= 13) ? 2 : (round >= 10 ? 2 : 1);
    const int eliteStar = (round >= 10) ? 2 : 1;

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

    const double armyHp = plannedArmyHp(cfg.spawnList, templates());
    if (armyHp > 0.0) {
        cfg.statScaleFactor = endlessTargetArmyHp(round) / armyHp;
    }

    // 失败惩罚和金币奖励随关数增长（上限分别为50和30）
    cfg.onLosePlayerHpDamage = std::min(50, 25 + (round - 6) * 3);
    cfg.winGoldReward        = std::min(30, 12 + (round - 6) * 2);
    return cfg;
}

// 流程：读取关卡配置 ──> 遍历 spawnList ──> 校验模板/位置 ──> 按 statScaleFactor 缩放属性 ──> 创建单位并落盘 ──> 收集已生成指针
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
        // 无尽关卡：对模板 hp/atk/physDef/magDef 统一应用 statScaleFactor 膨胀/削弱。
        EnemyTemplate scaledTpl = tpl.at(entry.templateIndex);
        if (std::abs(cfg.statScaleFactor - 1.0) > 1e-6) {
            scaledTpl.hp      = static_cast<int>(scaledTpl.hp      * cfg.statScaleFactor + 0.5);
            scaledTpl.atk     = static_cast<int>(scaledTpl.atk     * cfg.statScaleFactor + 0.5);
            scaledTpl.physDef = static_cast<int>(scaledTpl.physDef * cfg.statScaleFactor + 0.5);
            scaledTpl.magDef  = static_cast<int>(scaledTpl.magDef  * cfg.statScaleFactor + 0.5);
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
