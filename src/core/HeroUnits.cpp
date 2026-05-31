#include "core/HeroUnits.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace my_auto_arena {
namespace core {

namespace {

int percentOfStat(int stat, int percent) {
    return std::max(1, stat * percent / 100);
}

bool isOnSameLine(const Position& selfPos, const Position& unitPos, bool verticalLine) {
    if (verticalLine) {
        return unitPos.col == selfPos.col;
    }
    return unitPos.row == selfPos.row;
}

bool isVerticalLineTowardTarget(const Position& selfPos, const Position& targetPos) {
    const int dr = selfPos.row - targetPos.row;
    const int dc = selfPos.col - targetPos.col;
    return dr * dr >= dc * dc;
}

}  // namespace

// 战士：近战爆发，技能：单体眩晕2回合 + 物攻150%伤害。
AshRaiderHero::AshRaiderHero(int id, UnitOwner owner)
    : PhysicalAttackUnit(id, "战士", owner, 1600, 62, 1, 75, UnitClass::kWarrior) {
    setBasePhysicalDef(20);
    setBaseMagicDef(5);
}

AshRaiderHero::AshRaiderHero(const AshRaiderHero& other) : PhysicalAttackUnit(other) {}

// 流程：校验主目标 ──> 造成物攻150%伤害 ──> 眩晕2回合 ──> 清空法力
void AshRaiderHero::castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) {
    (void)board;
    (void)units;
    if (primaryTarget != nullptr && primaryTarget->isAlive()) {
        const int skillDmg = percentOfStat(physicalAtk(), 150);
        primaryTarget->takePhysicalDamage(skillDmg);
        primaryTarget->applyStun(2);
    }
    spendAllMana();
}

// 射手：远程输出（射程4），技能：直线 AOE，物攻200%伤害。
NightArcherHero::NightArcherHero(int id, UnitOwner owner)
    : PhysicalAttackUnit(id, "射手", owner, 1200, 60, 4, 75, UnitClass::kArcher) {
    setBasePhysicalDef(5);
    setBaseMagicDef(5);
}

NightArcherHero::NightArcherHero(const NightArcherHero& other) : PhysicalAttackUnit(other) {}

// 流程：定位自身 ──> 按主目标方向确定直线（同行或同列）──> 对直线上所有敌方造成物攻200%伤害 ──> 清空法力
void NightArcherHero::castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) {
    const Position selfPos = board.findUnitOnBoard(id());
    if (!board.inBounds(selfPos)) {
        spendAllMana();
        return;
    }

    bool verticalLine = false;
    if (primaryTarget != nullptr) {
        const Position tgtPos = board.findUnitOnBoard(primaryTarget->id());
        if (board.inBounds(tgtPos)) {
            verticalLine = isVerticalLineTowardTarget(selfPos, tgtPos);
        }
    }

    const int skillDmg = percentOfStat(physicalAtk(), 200);
    for (std::map<int, Unit*>::iterator it = units.begin(); it != units.end(); ++it) {
        Unit* other = it->second;
        if (other == nullptr || !other->isAlive() || other->owner() == owner()) {
            continue;
        }
        const Position otherPos = board.findUnitOnBoard(other->id());
        if (!board.inBounds(otherPos)) {
            continue;
        }
        if (!isOnSameLine(selfPos, otherPos, verticalLine)) {
            continue;
        }
        other->takePhysicalDamage(skillDmg);
    }
    spendAllMana();
}

// 重甲战士：坦克近战，范围伤害技能：对周围4相邻格敌方各造成220点伤害。
CurseHammerHero::CurseHammerHero(int id, UnitOwner owner)
    : PhysicalAttackUnit(id, "重甲战士", owner, 2600, 48, 1, 90, UnitClass::kTank) {
    setBasePhysicalDef(50);
    setBaseMagicDef(15);
}

CurseHammerHero::CurseHammerHero(const CurseHammerHero& other) : PhysicalAttackUnit(other) {}

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

// 法师：技能对攻击范围内所有敌方造成法攻200%伤害。
MistWitchHero::MistWitchHero(int id, UnitOwner owner)
    : MagicalAttackUnit(id, "法师", owner, 1000, 38, 3, 70, UnitClass::kMage) {
    setBasePhysicalDef(5);
    setBaseMagicDef(25);
}

MistWitchHero::MistWitchHero(const MistWitchHero& other) : MagicalAttackUnit(other) {}

// 流程：定位自身 ──> 遍历敌方 ──> 在攻击射程内则造成法攻200%伤害 ──> 清空法力
void MistWitchHero::castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) {
    (void)primaryTarget;
    const Position selfPos = board.findUnitOnBoard(id());
    if (!board.inBounds(selfPos)) {
        spendAllMana();
        return;
    }

    const int skillDmg = percentOfStat(magicAtk(), 200);
    const int rangeSq = attackRange() * attackRange();
    for (std::map<int, Unit*>::iterator it = units.begin(); it != units.end(); ++it) {
        Unit* other = it->second;
        if (other == nullptr || !other->isAlive() || other->owner() == owner()) {
            continue;
        }
        const Position otherPos = board.findUnitOnBoard(other->id());
        if (!board.inBounds(otherPos)) {
            continue;
        }
        const int dr = selfPos.row - otherPos.row;
        const int dc = selfPos.col - otherPos.col;
        if (dr * dr + dc * dc > rangeSq) {
            continue;
        }
        other->takeMagicDamage(skillDmg);
    }
    spendAllMana();
}

// 治疗师：为射程内全体友方（含自身）治疗 maxHp * 15%。
BonePrayerHero::BonePrayerHero(int id, UnitOwner owner)
    : MagicalAttackUnit(id, "治疗师", owner, 1400, 28, 3, 80, UnitClass::kHealer) {
    setBasePhysicalDef(5);
    setBaseMagicDef(20);
}

BonePrayerHero::BonePrayerHero(const BonePrayerHero& other) : MagicalAttackUnit(other) {}

void BonePrayerHero::castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) {
    (void)primaryTarget;
    const int healAmount = std::max(1, static_cast<int>(maxHp() * 0.15));
    const Position selfPos = board.findUnitOnBoard(id());
    const int r = attackRange();

    for (std::map<int, Unit*>::iterator it = units.begin(); it != units.end(); ++it) {
        Unit* ally = it->second;
        if (ally == nullptr || !ally->isAlive() || ally->owner() != owner()) {
            continue;
        }
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

const char* skillDescriptionForHeroType(HeroType type) {
    switch (type) {
        case HeroType::kWarrior:
            return "对单体目标造成物攻150%伤害并眩晕2回合";
        case HeroType::kArcher:
            return "对施法者所在直线（同行或同列）上所有敌方造成物攻200%伤害";
        case HeroType::kTank:
            return "对周围4相邻格内所有敌方造成220点物理伤害";
        case HeroType::kMage:
            return "对攻击范围内所有敌方造成法攻200%伤害";
        case HeroType::kHealer:
            return "在自身攻击射程内所有友方（含自己）治疗 自身maxHp × 15%";
        default:
            return "无技能描述";
    }
}

const char* skillDescriptionForUnitClass(UnitClass cls) {
    switch (cls) {
        case UnitClass::kWarrior: return skillDescriptionForHeroType(HeroType::kWarrior);
        case UnitClass::kArcher:  return skillDescriptionForHeroType(HeroType::kArcher);
        case UnitClass::kTank:    return skillDescriptionForHeroType(HeroType::kTank);
        case UnitClass::kMage:    return skillDescriptionForHeroType(HeroType::kMage);
        case UnitClass::kHealer:  return skillDescriptionForHeroType(HeroType::kHealer);
        default:                  return "无技能描述";
    }
}

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
