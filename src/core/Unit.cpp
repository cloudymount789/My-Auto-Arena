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
      maxHp_(maxHp),
      attack_(attack),
      attackRange_(attackRange),
      mana_(0),
      maxMana_(maxMana),
      unitClass_(unitClass),
      starLevel_(1),
      equippedItem_(ItemType::kNone),
      bonusAtk_(0),
      bonusMaxHp_(0),
      star1Atk_(attack),
      star1MaxHp_(maxHp) {
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
      maxMana_(other.maxMana_),
      unitClass_(other.unitClass_),
      starLevel_(other.starLevel_),
      equippedItem_(other.equippedItem_),
      bonusAtk_(other.bonusAtk_),
      bonusMaxHp_(other.bonusMaxHp_),
      star1Atk_(other.star1Atk_),
      star1MaxHp_(other.star1MaxHp_) {}

Unit& Unit::operator=(const Unit& other) {
    if (this == &other) {
        return *this;
    }
    id_ = other.id_;
    name_ = other.name_;
    owner_ = other.owner_;
    hp_ = other.hp_;
    maxHp_ = other.maxHp_;
    attack_ = other.attack_;
    attackRange_ = other.attackRange_;
    mana_ = other.mana_;
    maxMana_ = other.maxMana_;
    unitClass_ = other.unitClass_;
    starLevel_ = other.starLevel_;
    equippedItem_ = other.equippedItem_;
    bonusAtk_ = other.bonusAtk_;
    bonusMaxHp_ = other.bonusMaxHp_;
    star1Atk_ = other.star1Atk_;
    star1MaxHp_ = other.star1MaxHp_;
    return *this;
}

int Unit::id() const { return id_; }
const std::string& Unit::name() const { return name_; }
UnitOwner Unit::owner() const { return owner_; }
int Unit::hp() const { return hp_; }
// maxHp() 返回含羁绊加成的有效最大血量。
int Unit::maxHp() const { return maxHp_ + bonusMaxHp_; }
// attack() 返回含羁绊加成的有效攻击力。
int Unit::attack() const { return attack_ + bonusAtk_; }
int Unit::attackRange() const { return attackRange_; }
int Unit::mana() const { return mana_; }
int Unit::maxMana() const { return maxMana_; }
bool Unit::isAlive() const { return hp_ > 0; }

UnitClass Unit::unitClass() const { return unitClass_; }
int Unit::starLevel() const { return starLevel_; }
ItemType Unit::equippedItem() const { return equippedItem_; }

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
    // 上限为 maxHp()（含羁绊加成）。
    hp_ = std::min(maxHp(), hp_ + amount);
}

void Unit::spendAllMana() { mana_ = 0; }

void Unit::resetToFull() {
    // 含羁绊加成的完整血量重置。
    hp_ = maxHp_ + bonusMaxHp_;
    mana_ = 0;
}

void Unit::equipItem(ItemType item) {
    if (equippedItem_ != ItemType::kNone) {
        unequipItem();
    }
    const ItemDef& def = getItemDef(item);
    attack_ += def.bonusAtk;

    if (def.bonusMaxHp > 0) {
        // 装备增加最大血量时，按比例放大当前血量，避免穿甲后显示"受伤"状态。
        const int oldMax = maxHp();  // 含羁绊加成
        maxHp_ += def.bonusMaxHp;
        const int newMax = maxHp();
        // hp_ 按比例缩放，结果向上取整，保证至少为 1。
        hp_ = static_cast<int>(static_cast<double>(hp_) / oldMax * newMax + 0.5);
        hp_ = std::max(1, std::min(hp_, newMax));
    } else {
        maxHp_ += def.bonusMaxHp;
    }
    equippedItem_ = item;
}

void Unit::unequipItem() {
    if (equippedItem_ == ItemType::kNone) {
        return;
    }
    const ItemDef& def = getItemDef(equippedItem_);
    attack_ -= def.bonusAtk;
    maxHp_ -= def.bonusMaxHp;
    equippedItem_ = ItemType::kNone;
}

void Unit::setSynergyBuffs(int bonusAtk, int bonusMaxHp) {
    bonusAtk_ = bonusAtk;
    bonusMaxHp_ = bonusMaxHp;
}

void Unit::clearSynergyBuffs() {
    bonusAtk_ = 0;
    bonusMaxHp_ = 0;
}

void Unit::upgradeToStar(int newStarLevel) {
    // 计算当前装备加成，升星后需还原到装备值上。
    int itemAtk = (equippedItem_ != ItemType::kNone) ? getItemDef(equippedItem_).bonusAtk : 0;
    int itemHp  = (equippedItem_ != ItemType::kNone) ? getItemDef(equippedItem_).bonusMaxHp : 0;

    // 升星倍率：★2 = 3.0×，★3 = 7.0×；确保升星收益明显高于不升星。
    double factor = (newStarLevel == 2) ? 3.0 : 7.0;
    // 用原始星1基础值乘倍率，避免装备或之前升星导致的重复叠乘。
    attack_ = static_cast<int>(star1Atk_ * factor) + itemAtk;
    maxHp_  = static_cast<int>(star1MaxHp_ * factor) + itemHp;
    hp_     = maxHp_;  // 升星后满血
    starLevel_ = newStarLevel;
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
        primaryTarget->takeDamage(attack());
    }
}

void Unit::castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) {
    (void)board;
    (void)units;
    (void)primaryTarget;
    spendAllMana();
}

WarriorUnit::WarriorUnit(int id, UnitOwner owner) : Unit(id, "Warrior", owner, 800, 65, 1, 100) {}

WarriorUnit::WarriorUnit(const WarriorUnit& other) : Unit(other) {}

void WarriorUnit::castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) {
    (void)units;
    performAttackInRange(board, primaryTarget);
    spendAllMana();
}

MageUnit::MageUnit(int id, UnitOwner owner) : Unit(id, "Mage", owner, 500, 45, 3, 100) {}

MageUnit::MageUnit(const MageUnit& other) : Unit(other) {}

void MageUnit::castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) {
    (void)units;
    performAttackInRange(board, primaryTarget);
    spendAllMana();
}

}  // namespace core
}  // namespace my_auto_arena
