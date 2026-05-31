#include "core/Unit.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "core/Board.h"

namespace my_auto_arena {
namespace core {

namespace {

const int kBaseAttackSpeed = 100;

}  // namespace

Unit::Unit(int id, std::string name, UnitOwner owner, int maxHp, int attack, int attackRange, int maxMana,
           UnitClass unitClass)
    : id_(id),
      name_(std::move(name)),
      owner_(owner),
      hp_(maxHp),
      baseMaxHp_(maxHp),
      baseAttack_(attack),
      attackRange_(attackRange),
      mana_(0),
      maxMana_(maxMana),
      unitClass_(unitClass),
      starLevel_(1),
      equippedItems_(),
      bonusAtk_(0),
      bonusMagAtk_(0),
      bonusMaxHp_(0),
      star1Atk_(attack),
      star1MaxHp_(maxHp),
      star1MagAtk_(0),
      baseMagicAtk_(0),
      basePhysicalDef_(0),
      baseMagicDef_(0),
      star1PhysDef_(0),
      star1MagDef_(0),
      state_(UnitState::kIdle),
      currentPhysicalAtk_(attack),
      currentMagicAtk_(0),
      currentMaxHp_(maxHp),
      currentPhysicalDef_(0),
      currentMagicDef_(0),
      currentMaxMana_(maxMana),
      currentAttackSpeed_(kBaseAttackSpeed) {
    if (id < 0 || maxHp <= 0 || attack < 0 || attackRange <= 0 || maxMana <= 0) {
        throw std::invalid_argument("Invalid unit stats.");
    }
    recalculateCurrentStats();
    hp_ = currentMaxHp_;
}

Unit::Unit(const Unit& other)
    : id_(other.id_),
      name_(other.name_),
      owner_(other.owner_),
      hp_(other.hp_),
      baseMaxHp_(other.baseMaxHp_),
      baseAttack_(other.baseAttack_),
      attackRange_(other.attackRange_),
      mana_(other.mana_),
      maxMana_(other.maxMana_),
      unitClass_(other.unitClass_),
      starLevel_(other.starLevel_),
      equippedItems_(other.equippedItems_),
      bonusAtk_(other.bonusAtk_),
      bonusMagAtk_(other.bonusMagAtk_),
      bonusMaxHp_(other.bonusMaxHp_),
      star1Atk_(other.star1Atk_),
      star1MaxHp_(other.star1MaxHp_),
      star1MagAtk_(other.star1MagAtk_),
      baseMagicAtk_(other.baseMagicAtk_),
      basePhysicalDef_(other.basePhysicalDef_),
      baseMagicDef_(other.baseMagicDef_),
      star1PhysDef_(other.star1PhysDef_),
      star1MagDef_(other.star1MagDef_),
      state_(other.state_),
      currentPhysicalAtk_(other.currentPhysicalAtk_),
      currentMagicAtk_(other.currentMagicAtk_),
      currentMaxHp_(other.currentMaxHp_),
      currentPhysicalDef_(other.currentPhysicalDef_),
      currentMagicDef_(other.currentMagicDef_),
      currentMaxMana_(other.currentMaxMana_),
      currentAttackSpeed_(other.currentAttackSpeed_) {}

Unit& Unit::operator=(const Unit& other) {
    if (this == &other) {
        return *this;
    }
    id_ = other.id_;
    name_ = other.name_;
    owner_ = other.owner_;
    hp_ = other.hp_;
    baseMaxHp_ = other.baseMaxHp_;
    baseAttack_ = other.baseAttack_;
    attackRange_ = other.attackRange_;
    mana_ = other.mana_;
    maxMana_ = other.maxMana_;
    unitClass_ = other.unitClass_;
    starLevel_ = other.starLevel_;
    equippedItems_ = other.equippedItems_;
    bonusAtk_ = other.bonusAtk_;
    bonusMagAtk_ = other.bonusMagAtk_;
    bonusMaxHp_ = other.bonusMaxHp_;
    star1Atk_ = other.star1Atk_;
    star1MaxHp_ = other.star1MaxHp_;
    star1MagAtk_ = other.star1MagAtk_;
    baseMagicAtk_ = other.baseMagicAtk_;
    basePhysicalDef_ = other.basePhysicalDef_;
    baseMagicDef_ = other.baseMagicDef_;
    star1PhysDef_ = other.star1PhysDef_;
    star1MagDef_ = other.star1MagDef_;
    state_ = other.state_;
    currentPhysicalAtk_ = other.currentPhysicalAtk_;
    currentMagicAtk_ = other.currentMagicAtk_;
    currentMaxHp_ = other.currentMaxHp_;
    currentPhysicalDef_ = other.currentPhysicalDef_;
    currentMagicDef_ = other.currentMagicDef_;
    currentMaxMana_ = other.currentMaxMana_;
    currentAttackSpeed_ = other.currentAttackSpeed_;
    return *this;
}

int Unit::roundStat(double value) {
    return static_cast<int>(value + (value >= 0.0 ? 0.5 : -0.5));
}

int Unit::id() const { return id_; }
const std::string& Unit::name() const { return name_; }
UnitOwner Unit::owner() const { return owner_; }
int Unit::hp() const { return hp_; }
int Unit::maxHp() const { return currentMaxHp_; }
int Unit::attack() const { return physicalAtk(); }
int Unit::physicalAtk() const { return currentPhysicalAtk_; }
int Unit::magicAtk() const { return currentMagicAtk_; }
int Unit::physicalDef() const { return currentPhysicalDef_; }
int Unit::magicDef() const { return currentMagicDef_; }
int Unit::attackSpeed() const { return currentAttackSpeed_; }
int Unit::attackRange() const { return attackRange_; }
int Unit::mana() const { return mana_; }
int Unit::maxMana() const { return currentMaxMana_; }
bool Unit::isAlive() const { return hp_ > 0; }

int Unit::basePhysicalAtk() const { return baseAttack_; }
int Unit::baseMagicAtk() const { return baseMagicAtk_; }
int Unit::baseMaxHp() const { return baseMaxHp_; }
int Unit::basePhysicalDef() const { return basePhysicalDef_; }
int Unit::baseMagicDef() const { return baseMagicDef_; }
int Unit::baseMaxMana() const { return maxMana_; }
int Unit::baseAttackSpeed() const { return kBaseAttackSpeed; }

UnitClass Unit::unitClass() const { return unitClass_; }
int Unit::starLevel() const { return starLevel_; }
const std::vector<ItemType>& Unit::equippedItems() const { return equippedItems_; }
ItemType Unit::equippedItem() const {
    if (equippedItems_.empty()) {
        return ItemType::kNone;
    }
    return equippedItems_.at(0);
}
int Unit::equipSlotCount() const { return starLevel_; }

UnitState Unit::state() const { return state_; }
void Unit::setState(UnitState s) { state_ = s; }

void Unit::takeDamage(int amount) {
    if (amount <= 0 || !isAlive()) {
        return;
    }
    hp_ = std::max(0, hp_ - amount);
}

void Unit::takePhysicalDamage(int rawDmg) {
    const int net = std::max(1, rawDmg - physicalDef());
    takeDamage(net);
}

void Unit::takeMagicDamage(int rawDmg) {
    const int net = std::max(1, rawDmg - magicDef());
    takeDamage(net);
}

void Unit::gainMana(int amount) {
    if (amount <= 0 || !isAlive()) {
        return;
    }
    mana_ = std::min(maxMana(), mana_ + amount);
}

void Unit::heal(int amount) {
    if (amount <= 0 || !isAlive()) {
        return;
    }
    hp_ = std::min(maxHp(), hp_ + amount);
}

void Unit::spendAllMana() { mana_ = 0; }

void Unit::resetToFull() {
    recalculateCurrentStats();
    hp_ = maxHp();
    mana_ = 0;
}

void Unit::equipItem(ItemType item) {
    if (item == ItemType::kNone) {
        return;
    }

    const int oldMax = maxHp();
    if (static_cast<int>(equippedItems_.size()) < equipSlotCount()) {
        equippedItems_.push_back(item);
    } else {
        equippedItems_.at(0) = item;
    }

    recalculateCurrentStats();

    const int newMax = maxHp();
    if (newMax > oldMax && oldMax > 0 && hp_ > 0) {
        hp_ = roundStat(static_cast<double>(hp_) / oldMax * newMax);
    }
    clampHpToCurrentMax();
    clampManaToCurrentMax();
}

void Unit::unequipItem() { unequipItemAt(0); }

void Unit::unequipItemAt(int slotIndex) {
    if (slotIndex < 0 || slotIndex >= static_cast<int>(equippedItems_.size())) {
        return;
    }
    equippedItems_.erase(equippedItems_.begin() + slotIndex);
    recalculateCurrentStats();
    clampHpToCurrentMax();
    clampManaToCurrentMax();
}

std::vector<ItemType> Unit::takeAllEquippedItems() {
    std::vector<ItemType> items = equippedItems_;
    equippedItems_.clear();
    recalculateCurrentStats();
    clampHpToCurrentMax();
    clampManaToCurrentMax();
    return items;
}

void Unit::setSynergyBuffs(int bonusAtk, int bonusMagAtk, int bonusMaxHp) {
    bonusAtk_ = bonusAtk;
    bonusMagAtk_ = bonusMagAtk;
    bonusMaxHp_ = bonusMaxHp;
    recalculateCurrentStats();
    clampHpToCurrentMax();
}

void Unit::clearSynergyBuffs() {
    bonusAtk_ = 0;
    bonusMagAtk_ = 0;
    bonusMaxHp_ = 0;
    recalculateCurrentStats();
    clampHpToCurrentMax();
}

void Unit::upgradeToStar(int newStarLevel) {
    double factor = (newStarLevel == 2) ? 3.0 : 7.0;
    baseAttack_ = static_cast<int>(star1Atk_ * factor);
    baseMaxHp_ = static_cast<int>(star1MaxHp_ * factor);
    if (star1MagAtk_ > 0) {
        baseMagicAtk_ = static_cast<int>(star1MagAtk_ * factor);
    }
    if (star1PhysDef_ > 0) {
        basePhysicalDef_ = static_cast<int>(star1PhysDef_ * factor);
    }
    if (star1MagDef_ > 0) {
        baseMagicDef_ = static_cast<int>(star1MagDef_ * factor);
    }
    starLevel_ = newStarLevel;
    recalculateCurrentStats();
    hp_ = maxHp();
}

int Unit::scaledSkillDamage(int baseDamage) const {
    if (starLevel_ == 2) return static_cast<int>(baseDamage * 3.0);
    if (starLevel_ == 3) return static_cast<int>(baseDamage * 7.0);
    return baseDamage;
}

void Unit::performAttackInRange(Board& board, Unit* primaryTarget) {
    if (primaryTarget == nullptr || !primaryTarget->isAlive()) {
        return;
    }
    const Position selfPos = board.findUnitOnBoard(id());
    const Position tgtPos = board.findUnitOnBoard(primaryTarget->id());
    if (!board.inBounds(selfPos) || !board.inBounds(tgtPos)) {
        return;
    }
    const int dr = selfPos.row - tgtPos.row;
    const int dc = selfPos.col - tgtPos.col;
    const int r = attackRange();
    if (dr * dr + dc * dc <= r * r) {
        primaryTarget->takePhysicalDamage(physicalAtk());
    }
}

void Unit::castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) {
    (void)board;
    (void)units;
    (void)primaryTarget;
    spendAllMana();
}

int Unit::equipmentBonusPhysAtk() const {
    int total = 0;
    for (std::size_t i = 0; i < equippedItems_.size(); ++i) {
        const ItemDef& def = getItemDef(equippedItems_.at(i));
        total += roundStat(baseAttack_ * def.bonusPhysAtkPercent / 100.0);
    }
    return total;
}

int Unit::equipmentBonusMagAtk() const {
    int total = 0;
    for (std::size_t i = 0; i < equippedItems_.size(); ++i) {
        const ItemDef& def = getItemDef(equippedItems_.at(i));
        total += roundStat(baseMagicAtk_ * def.bonusMagAtkPercent / 100.0);
    }
    return total;
}

int Unit::equipmentBonusPhysDef() const {
    int total = 0;
    for (std::size_t i = 0; i < equippedItems_.size(); ++i) {
        const ItemDef& def = getItemDef(equippedItems_.at(i));
        total += roundStat(basePhysicalDef_ * def.bonusPhysDefensePercent / 100.0);
    }
    return total;
}

int Unit::equipmentBonusMagDef() const {
    int total = 0;
    for (std::size_t i = 0; i < equippedItems_.size(); ++i) {
        const ItemDef& def = getItemDef(equippedItems_.at(i));
        total += roundStat(baseMagicDef_ * def.bonusMagDefensePercent / 100.0);
    }
    return total;
}

int Unit::equipmentBonusMaxHp() const {
    int total = 0;
    for (std::size_t i = 0; i < equippedItems_.size(); ++i) {
        const ItemDef& def = getItemDef(equippedItems_.at(i));
        total += roundStat(baseMaxHp_ * def.bonusMaxHpPercent / 100.0);
    }
    return total;
}

int Unit::equipmentBonusAttackSpeed() const {
    int total = 0;
    for (std::size_t i = 0; i < equippedItems_.size(); ++i) {
        const ItemDef& def = getItemDef(equippedItems_.at(i));
        total += roundStat(kBaseAttackSpeed * def.bonusAttackSpeedPercent / 100.0);
    }
    return total;
}

int Unit::equipmentBonusMaxManaFlat() const {
    int total = 0;
    for (std::size_t i = 0; i < equippedItems_.size(); ++i) {
        total += getItemDef(equippedItems_.at(i)).bonusMaxManaFlat;
    }
    return total;
}

void Unit::recalculateCurrentStats() {
    const int equipPhysAtk = equipmentBonusPhysAtk();
    const int equipMagAtk = equipmentBonusMagAtk();
    const int equipMaxHp = equipmentBonusMaxHp();
    const int equipPhysDef = equipmentBonusPhysDef();
    const int equipMagDef = equipmentBonusMagDef();
    const int equipAtkSpeed = equipmentBonusAttackSpeed();

    currentPhysicalAtk_ = roundStat(baseAttack_ + equipPhysAtk + bonusAtk_);
    currentMagicAtk_ = roundStat(baseMagicAtk_ + equipMagAtk + bonusMagAtk_);
    currentMaxHp_ = roundStat(baseMaxHp_ + equipMaxHp + bonusMaxHp_);
    currentPhysicalDef_ = roundStat(basePhysicalDef_ + equipPhysDef);
    currentMagicDef_ = roundStat(baseMagicDef_ + equipMagDef);
    currentAttackSpeed_ = roundStat(kBaseAttackSpeed + equipAtkSpeed);

    const int rawMaxMana = maxMana_ + equipmentBonusMaxManaFlat();
    currentMaxMana_ = rawMaxMana < 1 ? 1 : rawMaxMana;
}

int Unit::equipmentMagicAtkBonus() const { return equipmentBonusMagAtk(); }

void Unit::setBaseMagicAtk(int v) {
    baseMagicAtk_ = v;
    star1MagAtk_ = v;
    recalculateCurrentStats();
}
void Unit::setBasePhysicalDef(int v) {
    basePhysicalDef_ = v;
    star1PhysDef_ = v;
    recalculateCurrentStats();
}
void Unit::setBaseMagicDef(int v) {
    baseMagicDef_ = v;
    star1MagDef_ = v;
    recalculateCurrentStats();
}

void Unit::clampHpToCurrentMax() {
    const int currentMaxHp = maxHp();
    if (hp_ > currentMaxHp) {
        hp_ = currentMaxHp;
    }
    if (hp_ < 0) {
        hp_ = 0;
    }
}

void Unit::clampManaToCurrentMax() {
    if (mana_ > maxMana()) {
        mana_ = maxMana();
    }
    if (mana_ < 0) {
        mana_ = 0;
    }
}

bool Unit::usesMagicAttack() const { return false; }

PhysicalAttackUnit::PhysicalAttackUnit(int id, const std::string& name, UnitOwner owner,
                                       int maxHp, int attack, int attackRange, int maxMana,
                                       UnitClass unitClass)
    : Unit(id, name, owner, maxHp, attack, attackRange, maxMana, unitClass) {}

PhysicalAttackUnit::PhysicalAttackUnit(const PhysicalAttackUnit& other) : Unit(other) {}

bool PhysicalAttackUnit::usesMagicAttack() const { return false; }

MagicalAttackUnit::MagicalAttackUnit(int id, const std::string& name, UnitOwner owner,
                                     int maxHp, int attack, int attackRange, int maxMana,
                                     UnitClass unitClass)
    : Unit(id, name, owner, maxHp, attack, attackRange, maxMana, unitClass) {
    setBaseMagicAtk(attack);
}

MagicalAttackUnit::MagicalAttackUnit(const MagicalAttackUnit& other) : Unit(other) {}

bool MagicalAttackUnit::usesMagicAttack() const { return true; }

WarriorUnit::WarriorUnit(int id, UnitOwner owner)
    : PhysicalAttackUnit(id, "Warrior", owner, 800, 65, 1, 100, UnitClass::kWarrior) {}

WarriorUnit::WarriorUnit(const WarriorUnit& other) : PhysicalAttackUnit(other) {}

void WarriorUnit::castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) {
    (void)units;
    performAttackInRange(board, primaryTarget);
    spendAllMana();
}

MageUnit::MageUnit(int id, UnitOwner owner)
    : MagicalAttackUnit(id, "Mage", owner, 500, 45, 3, 100, UnitClass::kMage) {}

MageUnit::MageUnit(const MageUnit& other) : MagicalAttackUnit(other) {}

void MageUnit::castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) {
    (void)units;
    performAttackInRange(board, primaryTarget);
    spendAllMana();
}

}  // namespace core
}  // namespace my_auto_arena
