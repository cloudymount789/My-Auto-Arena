#include "core/Unit.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "core/Board.h"

namespace my_auto_arena {
namespace core {

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
      state_(UnitState::kIdle) {
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
      state_(other.state_) {}

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
    return *this;
}

int Unit::id() const { return id_; }
const std::string& Unit::name() const { return name_; }
UnitOwner Unit::owner() const { return owner_; }
int Unit::hp() const { return hp_; }
// maxHp() 返回含装备与羁绊加成的有效最大血量。
int Unit::maxHp() const { return baseMaxHp_ + equipmentBonusMaxHp() + bonusMaxHp_; }
// attack() 向后兼容，等价于 physicalAtk()。
int Unit::attack() const { return physicalAtk(); }
// physicalAtk() = 基础物理攻击 + 装备物理攻加成 + 羁绊加成。
int Unit::physicalAtk() const { return baseAttack_ + equipmentBonusPhysAtk() + bonusAtk_; }
// magicAtk() = 基础法术攻击 + 装备法术攻加成 + 羁绊法术加成（法师专用）。
int Unit::magicAtk() const { return baseMagicAtk_ + equipmentBonusMagAtk() + bonusMagAtk_; }
// physicalDef() = 基础物理防御 + 装备物防加成。
int Unit::physicalDef() const { return basePhysicalDef_ + equipmentBonusPhysDef(); }
// magicDef() = 基础法术防御 + 装备魔防加成。
int Unit::magicDef() const { return baseMagicDef_ + equipmentBonusMagDef(); }
int Unit::attackRange() const { return attackRange_; }
int Unit::mana() const { return mana_; }
int Unit::maxMana() const { return maxMana_; }
bool Unit::isAlive() const { return hp_ > 0; }

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
    // 物理防御做平减，保底造成 1 点伤害。
    const int net = std::max(1, rawDmg - physicalDef());
    takeDamage(net);
}

void Unit::takeMagicDamage(int rawDmg) {
    // 法术防御做平减，保底造成 1 点伤害。
    const int net = std::max(1, rawDmg - magicDef());
    takeDamage(net);
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
    // 上限为 maxHp()（含羁绊加成）。
    hp_ = std::min(maxHp(), hp_ + amount);
}

void Unit::spendAllMana() { mana_ = 0; }

void Unit::resetToFull() {
    // 含装备与羁绊加成的完整血量重置。
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
        // 与旧行为兼容：槽位已满时，新装备替换第一个槽位。
        equippedItems_.at(0) = item;
    }

    const int newMax = maxHp();
    if (newMax > oldMax && oldMax > 0 && hp_ > 0) {
        // 最大生命值提高时按比例放大当前生命，保持血量百分比。
        hp_ = static_cast<int>(static_cast<double>(hp_) / oldMax * newMax + 0.5);
    }
    clampHpToCurrentMax();
}

void Unit::unequipItem() { unequipItemAt(0); }

void Unit::unequipItemAt(int slotIndex) {
    if (slotIndex < 0 || slotIndex >= static_cast<int>(equippedItems_.size())) {
        return;
    }
    equippedItems_.erase(equippedItems_.begin() + slotIndex);
    clampHpToCurrentMax();
}

void Unit::setSynergyBuffs(int bonusAtk, int bonusMagAtk, int bonusMaxHp) {
    bonusAtk_ = bonusAtk;
    bonusMagAtk_ = bonusMagAtk;
    bonusMaxHp_ = bonusMaxHp;
}

void Unit::clearSynergyBuffs() {
    bonusAtk_ = 0;
    bonusMagAtk_ = 0;
    bonusMaxHp_ = 0;
}

void Unit::upgradeToStar(int newStarLevel) {
    // 升星倍率：★2 = 3.0×，★3 = 7.0×；确保升星收益明显高于不升星。
    double factor = (newStarLevel == 2) ? 3.0 : 7.0;
    // 只提升基础属性；装备加成通过 getter 统一叠加，避免状态错乱。
    baseAttack_   = static_cast<int>(star1Atk_    * factor);
    baseMaxHp_    = static_cast<int>(star1MaxHp_  * factor);
    // 法师的法术攻击随星级等比放大（star1MagAtk_ 非零时生效）。
    if (star1MagAtk_ > 0) {
        baseMagicAtk_ = static_cast<int>(star1MagAtk_ * factor);
    }
    // 防御值随星级等比放大（非零时生效，保留0值单位不变）。
    if (star1PhysDef_ > 0) {
        basePhysicalDef_ = static_cast<int>(star1PhysDef_ * factor);
    }
    if (star1MagDef_ > 0) {
        baseMagicDef_ = static_cast<int>(star1MagDef_ * factor);
    }
    starLevel_ = newStarLevel;
    hp_ = maxHp();  // 升星后满血
}

int Unit::scaledSkillDamage(int baseDamage) const {
    if (starLevel_ == 2) return static_cast<int>(baseDamage * 3.0);
    if (starLevel_ == 3) return static_cast<int>(baseDamage * 7.0);
    return baseDamage;  // ★1 不缩放
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
        total += getItemDef(equippedItems_.at(i)).bonusPhysAtk;
    }
    return total;
}

int Unit::equipmentBonusMagAtk() const {
    int total = 0;
    for (std::size_t i = 0; i < equippedItems_.size(); ++i) {
        total += getItemDef(equippedItems_.at(i)).bonusMagAtk;
    }
    return total;
}

int Unit::equipmentBonusPhysDef() const {
    int total = 0;
    for (std::size_t i = 0; i < equippedItems_.size(); ++i) {
        total += getItemDef(equippedItems_.at(i)).bonusPhysDefense;
    }
    return total;
}

int Unit::equipmentBonusMagDef() const {
    int total = 0;
    for (std::size_t i = 0; i < equippedItems_.size(); ++i) {
        total += getItemDef(equippedItems_.at(i)).bonusMagDefense;
    }
    return total;
}

int Unit::equipmentBonusMaxHp() const {
    int total = 0;
    for (std::size_t i = 0; i < equippedItems_.size(); ++i) {
        total += getItemDef(equippedItems_.at(i)).bonusMaxHp;
    }
    return total;
}

void Unit::setBaseMagicAtk(int v) {
    baseMagicAtk_ = v;
    star1MagAtk_  = v;  // 同步记录原始值，供 upgradeToStar() 按比例缩放
}
void Unit::setBasePhysicalDef(int v) {
    basePhysicalDef_ = v;
    star1PhysDef_ = v;  // 同步记录原始值，供 upgradeToStar() 按比例缩放
}
void Unit::setBaseMagicDef(int v) {
    baseMagicDef_ = v;
    star1MagDef_ = v;   // 同步记录原始值，供 upgradeToStar() 按比例缩放
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

// ─────────────────────────────────────────────────────────────────────────────
// Unit::usesMagicAttack() 默认实现（物理攻击单位）
// ─────────────────────────────────────────────────────────────────────────────
bool Unit::usesMagicAttack() const { return false; }

// ─────────────────────────────────────────────────────────────────────────────
// PhysicalAttackUnit — 物理攻击中间层
// ─────────────────────────────────────────────────────────────────────────────
PhysicalAttackUnit::PhysicalAttackUnit(int id, const std::string& name, UnitOwner owner,
                                       int maxHp, int attack, int attackRange, int maxMana,
                                       UnitClass unitClass)
    : Unit(id, name, owner, maxHp, attack, attackRange, maxMana, unitClass) {}

PhysicalAttackUnit::PhysicalAttackUnit(const PhysicalAttackUnit& other) : Unit(other) {}

bool PhysicalAttackUnit::usesMagicAttack() const { return false; }

// ─────────────────────────────────────────────────────────────────────────────
// MagicalAttackUnit — 法术攻击中间层
// 构造时自动调用 setBaseMagicAtk(attack)，将 attack 参数注入 baseMagicAtk_。
// ─────────────────────────────────────────────────────────────────────────────
MagicalAttackUnit::MagicalAttackUnit(int id, const std::string& name, UnitOwner owner,
                                     int maxHp, int attack, int attackRange, int maxMana,
                                     UnitClass unitClass)
    : Unit(id, name, owner, maxHp, attack, attackRange, maxMana, unitClass) {
    setBaseMagicAtk(attack);
}

MagicalAttackUnit::MagicalAttackUnit(const MagicalAttackUnit& other) : Unit(other) {}

bool MagicalAttackUnit::usesMagicAttack() const { return true; }

// ─────────────────────────────────────────────────────────────────────────────
// WarriorUnit / MageUnit（教学用基础单位，继承自对应中间层）
// ─────────────────────────────────────────────────────────────────────────────
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
