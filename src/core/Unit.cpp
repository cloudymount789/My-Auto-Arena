#include "core/Unit.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "core/Board.h"

namespace my_auto_arena {
namespace core {

Unit::Unit(int id, std::string name, UnitOwner owner, int maxHp, int attack, int attackRange, int maxMana)
    : id_(id),
      name_(std::move(name)),
      owner_(owner),
      hp_(maxHp),
      maxHp_(maxHp),
      attack_(attack),
      attackRange_(attackRange),
      mana_(0),
      maxMana_(maxMana) {
    // 允许 attack == 0，用于后续可能的纯辅助类单位。
    if (id < 0 || maxHp <= 0 || attack < 0 || attackRange <= 0 || maxMana <= 0) {
        throw std::invalid_argument("Invalid unit stats.");
    }
}

Unit::Unit(const Unit& other)
    : id_(other.id_),
      name_(other.name_),
      owner_(other.owner_),
      hp_(other.hp_),
      maxHp_(other.maxHp_),
      attack_(other.attack_),
      attackRange_(other.attackRange_),
      mana_(other.mana_),
      maxMana_(other.maxMana_) {}

int Unit::id() const { return id_; }

const std::string& Unit::name() const { return name_; }

UnitOwner Unit::owner() const { return owner_; }

int Unit::hp() const { return hp_; }

int Unit::maxHp() const { return maxHp_; }

int Unit::attack() const { return attack_; }

int Unit::attackRange() const { return attackRange_; }

int Unit::mana() const { return mana_; }

int Unit::maxMana() const { return maxMana_; }

bool Unit::isAlive() const { return hp_ > 0; }

void Unit::takeDamage(int amount) {
    if (amount <= 0 || !isAlive()) {
        return;
    }
    hp_ = std::max(0, hp_ - amount);
}

void Unit::gainMana(int amount) {
    if (amount <= 0 || !isAlive()) {
        return;
    }
    mana_ = std::min(maxMana_, mana_ + amount);
}

void Unit::heal(int amount) {
    if (amount <= 0 || !isAlive()) {
        return;
    }
    hp_ = std::min(maxHp_, hp_ + amount);
}

void Unit::spendAllMana() { mana_ = 0; }

void Unit::castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) {
    (void)board;
    (void)units;
    (void)primaryTarget;
    spendAllMana();
}

WarriorUnit::WarriorUnit(int id, UnitOwner owner) : Unit(id, "Warrior", owner, 800, 65, 1, 100) {}

void WarriorUnit::castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) {
    (void)units;
    if (primaryTarget != nullptr && primaryTarget->isAlive()) {
        const Position selfPos = board.findUnitOnBoard(id());
        const Position tgtPos = board.findUnitOnBoard(primaryTarget->id());
        if (board.inBounds(selfPos) && board.inBounds(tgtPos)) {
            const int dr = selfPos.row - tgtPos.row;
            const int dc = selfPos.col - tgtPos.col;
            const int r = attackRange();
            if (dr * dr + dc * dc <= r * r) {
                primaryTarget->takeDamage(attack());
            }
        }
    }
    spendAllMana();
}

MageUnit::MageUnit(int id, UnitOwner owner) : Unit(id, "Mage", owner, 500, 45, 3, 100) {}

void MageUnit::castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) {
    (void)units;
    if (primaryTarget != nullptr && primaryTarget->isAlive()) {
        const Position selfPos = board.findUnitOnBoard(id());
        const Position tgtPos = board.findUnitOnBoard(primaryTarget->id());
        if (board.inBounds(selfPos) && board.inBounds(tgtPos)) {
            const int dr = selfPos.row - tgtPos.row;
            const int dc = selfPos.col - tgtPos.col;
            const int r = attackRange();
            if (dr * dr + dc * dc <= r * r) {
                primaryTarget->takeDamage(attack());
            }
        }
    }
    spendAllMana();
}

}  // namespace core
}  // namespace my_auto_arena
