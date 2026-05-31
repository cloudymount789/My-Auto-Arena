#include "core/HeroUnits.h"

#include <algorithm>
#include <stdexcept>

namespace my_auto_arena {
namespace core {

// 战士：近战爆发，生命值厚实，技能：对主目标造成 280 点爆发伤害。
// 设计定位：前排冲锋，削减血量可让前期压力更明显。
AshRaiderHero::AshRaiderHero(int id, UnitOwner owner)
    : PhysicalAttackUnit(id, "战士", owner, 1600, 62, 1, 75, UnitClass::kWarrior) {
    setBasePhysicalDef(20);   // 轻甲前排，物防中等
    setBaseMagicDef(5);
}

AshRaiderHero::AshRaiderHero(const AshRaiderHero& other) : PhysicalAttackUnit(other) {}

// 流程：校验主目标存活 ──> 造成缩放物理爆发伤害 ──> 清空法力
void AshRaiderHero::castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) {
    (void)board;
    (void)units;
    if (primaryTarget != nullptr && primaryTarget->isAlive()) {
        primaryTarget->takePhysicalDamage(scaledSkillDamage(280));
    }
    spendAllMana();
}

// 射手：远程输出（射程 4），技能：对主目标造成 360 点穿透伤害。
// 设计定位：输出核心，弓手羁绊可极大提升其伤害，但HP偏低须保护。
NightArcherHero::NightArcherHero(int id, UnitOwner owner)
    : PhysicalAttackUnit(id, "射手", owner, 1200, 60, 4, 75, UnitClass::kArcher) {
    setBasePhysicalDef(5);    // 皮甲远程，两防均低
    setBaseMagicDef(5);
}

NightArcherHero::NightArcherHero(const NightArcherHero& other) : PhysicalAttackUnit(other) {}

// 流程：校验主目标存活 ──> 造成缩放物理穿透伤害 ──> 清空法力
void NightArcherHero::castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) {
    (void)board;
    (void)units;
    if (primaryTarget != nullptr && primaryTarget->isAlive()) {
        primaryTarget->takePhysicalDamage(scaledSkillDamage(360));
    }
    spendAllMana();
}

// 重甲战士：坦克近战，范围伤害技能：对周围 4 相邻格敌方各造成 220 点伤害。
// 设计定位：肉盾+范围伤害，近战羁绊为其解锁高物攻加成，但技能转换慢。
CurseHammerHero::CurseHammerHero(int id, UnitOwner owner)
    : PhysicalAttackUnit(id, "重甲战士", owner, 2600, 48, 1, 90, UnitClass::kTank) {
    setBasePhysicalDef(50);   // 重甲肉盾，物防高
    setBaseMagicDef(15);      // 魔防中等
}

CurseHammerHero::CurseHammerHero(const CurseHammerHero& other) : PhysicalAttackUnit(other) {}

// 流程：定位自身棋盘格 ──> 遍历四邻格 ──> 查 occupant 映射到 Unit ──> 对存活敌方造成范围物伤 ──> 清空法力
void CurseHammerHero::castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) {
    (void)primaryTarget;
    const Position selfPos = board.findUnitOnBoard(id());
    if (!board.inBounds(selfPos)) {
        spendAllMana();
        return;
    }
    const int dirs[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    for (int d = 0; d < 4; ++d) {
        const Position nbr{selfPos.row + dirs[d][0], selfPos.col + dirs[d][1]};
        if (!board.inBounds(nbr)) {
            continue;
        }
        const int occ = board.occupantOnBoard(nbr);
        if (occ == Board::kEmptySlot) {
            continue;
        }
        std::map<int, Unit*>::iterator it = units.find(occ);
        if (it == units.end() || it->second == nullptr) {
            continue;
        }
        Unit* other = it->second;
        if (other->isAlive() && other->owner() != owner()) {
            other->takePhysicalDamage(scaledSkillDamage(220));
        }
    }
    spendAllMana();
}

// 法师：中程法术输出，普攻与技能均为法术伤害（继承 MagicalAttackUnit）。
// MagicalAttackUnit 构造时自动调用 setBaseMagicAtk(38)，无需手动设置。
// 设计定位：对高物防低魔防敌人（如重甲战士/攻城弩）有显著优势；须有坦克掩护。
MistWitchHero::MistWitchHero(int id, UnitOwner owner)
    : MagicalAttackUnit(id, "法师", owner, 1000, 38, 3, 70, UnitClass::kMage) {
    setBasePhysicalDef(5);    // 布甲，物防低
    setBaseMagicDef(25);      // 法术护盾，魔防高
}

MistWitchHero::MistWitchHero(const MistWitchHero& other) : MagicalAttackUnit(other) {}

// 流程：校验主目标存活 ──> 计算缩放法伤+装备法攻加成 ──> 对主目标造成法术伤害 ──> 清空法力
void MistWitchHero::castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) {
    (void)board;
    (void)units;
    if (primaryTarget != nullptr && primaryTarget->isAlive()) {
        // 技能伤害 = 基础爆发(随星级缩放) + 装备法术攻加成（魔纹环可直接提升技能威力）。
        const int skillDmg = scaledSkillDamage(420) + equipmentMagicAtkBonus();
        primaryTarget->takeMagicDamage(skillDmg);
    }
    spendAllMana();
}

// 治疗师：辅助单位，普攻也为法术伤害（继承 MagicalAttackUnit）。
// 技能：为射程内全体友方（含自身）治疗自身 maxHp * 15%。
// 疗愈符（+800 maxHp）直接提升治疗量：1400 maxHp → 210/次，2200 maxHp → 330/次。
// 设计定位：圣愈羁绊叠双治疗师可翻盘持久战；治疗量与 maxHp 装备正相关。
BonePrayerHero::BonePrayerHero(int id, UnitOwner owner)
    : MagicalAttackUnit(id, "治疗师", owner, 1400, 28, 3, 80, UnitClass::kHealer) {
    setBasePhysicalDef(5);    // 法系辅助，物防低
    setBaseMagicDef(20);      // 魔防较高
}

BonePrayerHero::BonePrayerHero(const BonePrayerHero& other) : MagicalAttackUnit(other) {}

// 流程：按 maxHp 算治疗量 ──> 遍历友方单位 ──> 若在棋盘上则校验射程 ──> 对范围内友方（含自身）治疗 ──> 清空法力
void BonePrayerHero::castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) {
    (void)primaryTarget;
    // 每次技能治疗量 = 自身 maxHp 的 15%（含装备加成）。
    const int healAmount = std::max(1, static_cast<int>(maxHp() * 0.15));
    const Position selfPos = board.findUnitOnBoard(id());
    const int r = attackRange();

    for (std::map<int, Unit*>::iterator it = units.begin(); it != units.end(); ++it) {
        Unit* ally = it->second;
        if (ally == nullptr || !ally->isAlive() || ally->owner() != owner()) {
            continue;
        }
        // 如果治疗师在棋盘上，检查距离；否则仅治疗自身。
        if (board.inBounds(selfPos)) {
            const Position allyPos = board.findUnitOnBoard(ally->id());
            if (!board.inBounds(allyPos)) {
                continue;
            }
            const int dr = selfPos.row - allyPos.row;
            const int dc = selfPos.col - allyPos.col;
            if (dr * dr + dc * dc > r * r) {
                continue;
            }
        } else if (ally->id() != id()) {
            continue;
        }
        ally->heal(scaledSkillDamage(healAmount));
    }
    spendAllMana();
}

// 流程：按 HeroType 分支 ──> new 对应英雄子类 ──> 未知类型抛 invalid_argument
Unit* createHero(HeroType type, int id, UnitOwner owner) {
    switch (type) {
        case HeroType::kWarrior: return new AshRaiderHero(id, owner);
        case HeroType::kArcher:  return new NightArcherHero(id, owner);
        case HeroType::kTank:    return new CurseHammerHero(id, owner);
        case HeroType::kMage:    return new MistWitchHero(id, owner);
        case HeroType::kHealer:  return new BonePrayerHero(id, owner);
        default:
            throw std::invalid_argument("未知英雄类型");
    }
}

}  // namespace core
}  // namespace my_auto_arena
