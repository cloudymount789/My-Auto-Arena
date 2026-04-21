#include "core/HeroUnits.h"

namespace my_auto_arena {
namespace core {

AshRaiderHero::AshRaiderHero(int id, UnitOwner owner)
    : Unit(id, "灰烬掠袭者", owner, 680, 70, 1, 65) {}

void AshRaiderHero::castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) {
    (void)board;
    (void)units;
    if (primaryTarget != nullptr && primaryTarget->isAlive()) {
        primaryTarget->takeDamage(180);
    }
    spendAllMana();
}

NightArcherHero::NightArcherHero(int id, UnitOwner owner)
    : Unit(id, "夜羽猎弓手", owner, 610, 74, 4, 70) {}

void NightArcherHero::castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) {
    (void)board;
    (void)units;
    if (primaryTarget != nullptr && primaryTarget->isAlive()) {
        primaryTarget->takeDamage(210);
    }
    spendAllMana();
}

CurseHammerHero::CurseHammerHero(int id, UnitOwner owner)
    : Unit(id, "诅印重锤奴", owner, 980, 54, 1, 95) {}

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
            other->takeDamage(120);
        }
    }
    spendAllMana();
}

MistWitchHero::MistWitchHero(int id, UnitOwner owner)
    : Unit(id, "瘴雾魔女学徒", owner, 620, 44, 3, 80) {}

void MistWitchHero::castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) {
    (void)board;
    (void)units;
    if (primaryTarget != nullptr && primaryTarget->isAlive()) {
        primaryTarget->takeDamage(90);
    }
    spendAllMana();
}

BonePrayerHero::BonePrayerHero(int id, UnitOwner owner)
    : Unit(id, "骨契祷告者", owner, 640, 38, 3, 85) {}

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
        best->heal(150);
    } else {
        heal(120);
    }
    (void)board;
    spendAllMana();
}

}  // namespace core
}  // namespace my_auto_arena
