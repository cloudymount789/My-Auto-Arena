#ifndef MY_AUTO_ARENA_CORE_UNIT_H
#define MY_AUTO_ARENA_CORE_UNIT_H

#include <map>
#include <string>

#include "core/Item.h"

namespace my_auto_arena {
namespace core {

class Board;

enum class UnitOwner { player, enemy };

// 职业分类：用于羁绊系统判断单位类型。
enum class UnitClass { kNone, kWarrior, kArcher, kTank, kMage, kHealer };

// 单位战斗状态机，对应规格书 Section 3.1 中的 S = {Idle, Moving, Attacking, Casting, Dead}。
enum class UnitState { kIdle, kMoving, kAttacking, kCasting, kDead };

class Unit {
public:
    // 战斗单位基类：管理通用属性与基础战斗行为。
    Unit(int id, std::string name, UnitOwner owner, int maxHp, int attack, int attackRange, int maxMana,
         UnitClass unitClass = UnitClass::kNone);
    // 按课程要求显式定义拷贝构造函数，便于讲解对象复制语义。
    Unit(const Unit& other);
    // 按 Rule of Three 要求配套定义拷贝赋值运算符。
    Unit& operator=(const Unit& other);
    virtual ~Unit() = default;

    int id() const;
    const std::string& name() const;
    UnitOwner owner() const;
    int hp() const;
    // maxHp() 含羁绊加成（bonusMaxHp_），不含装备（装备已直接加入 maxHp_）。
    int maxHp() const;
    // attack() 含羁绊加成（bonusAtk_），不含装备（装备已直接加入 attack_）。
    int attack() const;
    int attackRange() const;
    int mana() const;
    int maxMana() const;
    bool isAlive() const;

    // Phase 3 新增：职业、星级、装备 getter。
    UnitClass unitClass() const;
    int starLevel() const;
    ItemType equippedItem() const;

    // 战斗状态：由 BattleEngine 在每 tick 中更新，用于 GUI 状态展示与验收。
    UnitState state() const;
    void setState(UnitState s);

    void takeDamage(int amount);
    void gainMana(int amount);
    // heal() 上限为 maxHp()（含羁绊加成）。
    void heal(int amount);
    // resetToFull() 将 hp_ 重置为 maxHp_+bonusMaxHp_，蓝量归零。
    void resetToFull();

    // 装备管理：将装备属性直接叠加到 attack_/maxHp_；unequip 时减去。
    void equipItem(ItemType item);
    void unequipItem();

    // 羁绊 BUFF：每轮战斗开始前设置，结束后清除。
    void setSynergyBuffs(int bonusAtk, int bonusMaxHp);
    void clearSynergyBuffs();

    // 升星：用原始星1基础值乘以倍率（star2=×3.0，star3=×7.0），保留装备加成。
    void upgradeToStar(int newStarLevel);

    // 按星级缩放技能基础伤害/治疗量：★1=×1.0，★2=×3.0，★3=×7.0（与 ATK 倍率一致）。
    int scaledSkillDamage(int baseDamage) const;

    // 法力满时由战斗引擎调用：多态技能入口；默认仅清空法力。
    virtual void castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget);

protected:
    void spendAllMana();
    // 范围内对单目标造成普攻伤害；WarriorUnit/MageUnit 的默认技能共用此逻辑。
    void performAttackInRange(Board& board, Unit* primaryTarget);

private:
    int id_;
    std::string name_;
    UnitOwner owner_;
    int hp_;
    int maxHp_;       // 含装备加成，不含羁绊加成
    int attack_;      // 含装备加成，不含羁绊加成
    int attackRange_;
    int mana_;
    int maxMana_;

    UnitClass unitClass_;
    int starLevel_;
    ItemType equippedItem_;
    int bonusAtk_;    // 羁绊系统临时加成
    int bonusMaxHp_;  // 羁绊系统临时加成
    int star1Atk_;    // 原始星级1攻击力（升星乘算用）
    int star1MaxHp_;  // 原始星级1血量（升星乘算用）
    int hpBeforeEquip_;  // 穿戴加血装备前的 hp_，卸装时原样恢复
    UnitState state_;    // 战斗状态机当前状态，每 tick 由 BattleEngine 更新
};

class WarriorUnit final : public Unit {
public:
    WarriorUnit(int id, UnitOwner owner);
    // 显式拷贝构造函数：委托给基类 Unit，演示继承体系中的拷贝语义。
    WarriorUnit(const WarriorUnit& other);
    virtual ~WarriorUnit() override = default;

    virtual void castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) override;
};

class MageUnit final : public Unit {
public:
    MageUnit(int id, UnitOwner owner);
    // 显式拷贝构造函数：委托给基类 Unit，演示继承体系中的拷贝语义。
    MageUnit(const MageUnit& other);
    virtual ~MageUnit() override = default;

    virtual void castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) override;
};

}  // namespace core
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_CORE_UNIT_H
