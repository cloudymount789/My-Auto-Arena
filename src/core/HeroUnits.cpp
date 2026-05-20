#include "core/HeroUnits.h"

#include <stdexcept>

namespace my_auto_arena {
namespace core {

// 战士：近战爆发，HP 厚实，技能：对主目标造成 300 点爆发伤害。
AshRaiderHero::AshRaiderHero(int id, UnitOwner owner)
    : Unit(id, "战士", owner, 2200, 60, 1, 80, UnitClass::kWarrior) {}

AshRaiderHero::AshRaiderHero(const AshRaiderHero& other) : Unit(other) {}

void AshRaiderHero::castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) {
    (void)board;
    (void)units;
    if (primaryTarget != nullptr && primaryTarget->isAlive()) {
        primaryTarget->takeDamage(300);
    }
    spendAllMana();
}

// 射手：远程输出（射程 4），技能：对主目标造成 320 点穿透伤害。
NightArcherHero::NightArcherHero(int id, UnitOwner owner)
    : Unit(id, "射手", owner, 1800, 55, 4, 90, UnitClass::kArcher) {}

NightArcherHero::NightArcherHero(const NightArcherHero& other) : Unit(other) {}

void NightArcherHero::castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) {
    (void)board;
    (void)units;
    if (primaryTarget != nullptr && primaryTarget->isAlive()) {
        primaryTarget->takeDamage(320);
    }
    spendAllMana();
}

// 重甲战士：坦克近战，AOE 技能：对周围 4 相邻格敌方各造成 180 点伤害。
CurseHammerHero::CurseHammerHero(int id, UnitOwner owner)
    : Unit(id, "重甲战士", owner, 3200, 48, 1, 100, UnitClass::kTank) {}

CurseHammerHero::CurseHammerHero(const CurseHammerHero& other) : Unit(other) {}

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
            other->takeDamage(180);
        }
    }
    spendAllMana();
}

// 法师：中程法术输出，技能：对主目标造成 350 点法术伤害。
MistWitchHero::MistWitchHero(int id, UnitOwner owner)
    : Unit(id, "法师", owner, 1500, 35, 3, 90, UnitClass::kMage) {}

MistWitchHero::MistWitchHero(const MistWitchHero& other) : Unit(other) {}

void MistWitchHero::castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) {
    (void)board;
    (void)units;
    if (primaryTarget != nullptr && primaryTarget->isAlive()) {
        primaryTarget->takeDamage(350);
    }
    spendAllMana();
}

// 治疗师：辅助单位，技能：为血量最低的友方治疗 500 点；无其他友方则自愈 400 点。
BonePrayerHero::BonePrayerHero(int id, UnitOwner owner)
    : Unit(id, "治疗师", owner, 1800, 30, 3, 100, UnitClass::kHealer) {}

BonePrayerHero::BonePrayerHero(const BonePrayerHero& other) : Unit(other) {}

void BonePrayerHero::castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) {
    (void)primaryTarget;
    Unit* best = nullptr;
    for (std::map<int, Unit*>::iterator it = units.begin(); it != units.end(); ++it) {
        Unit* ally = it->second;
        if (ally == nullptr || !ally->isAlive() || ally->owner() != owner()) {
            continue;
        }
        if (ally->id() == id()) {
            continue;
        }
        if (best == nullptr || ally->hp() < best->hp()) {
            best = ally;
        }
    }
    if (best != nullptr) {
        best->heal(500);
    } else {
        heal(400);
    }
    (void)board;
    spendAllMana();
}

// 英雄工厂：根据 HeroType 创建堆分配的英雄实例。
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
