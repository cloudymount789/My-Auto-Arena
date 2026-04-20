#include "core/Unit.h"

#include <algorithm>
#include <stdexcept>

namespace my_auto_arena::core {

Unit::Unit(const int id, std::string name, const UnitOwner owner, const int max_hp, const int attack,
           const int attack_range, const int max_mana)
    : id_(id),
      name_(std::move(name)),
      owner_(owner),
      hp_(max_hp),
      max_hp_(max_hp),
      attack_(attack),
      attack_range_(attack_range),
      max_mana_(max_mana) {
    if (id < 0 || max_hp <= 0 || attack < 0 || attack_range <= 0 || max_mana <= 0) {
        throw std::invalid_argument("Invalid unit stats.");
    }
}

int Unit::id() const noexcept { return id_; }

const std::string& Unit::name() const noexcept { return name_; }

UnitOwner Unit::owner() const noexcept { return owner_; }

int Unit::hp() const noexcept { return hp_; }

int Unit::max_hp() const noexcept { return max_hp_; }

int Unit::attack() const noexcept { return attack_; }

int Unit::attack_range() const noexcept { return attack_range_; }

int Unit::mana() const noexcept { return mana_; }

int Unit::max_mana() const noexcept { return max_mana_; }

bool Unit::is_alive() const noexcept { return hp_ > 0; }

void Unit::take_damage(const int amount) noexcept {
    if (amount <= 0 || !is_alive()) {
        return;
    }
    hp_ = std::max(0, hp_ - amount);
}

void Unit::gain_mana(const int amount) noexcept {
    if (amount <= 0 || !is_alive()) {
        return;
    }
    mana_ = std::min(max_mana_, mana_ + amount);
}

WarriorUnit::WarriorUnit(const int id, const UnitOwner owner) : Unit(id, "Warrior", owner, 800, 65, 1, 100) {}

MageUnit::MageUnit(const int id, const UnitOwner owner) : Unit(id, "Mage", owner, 500, 45, 3, 100) {}

}  // namespace my_auto_arena::core
