#ifndef MY_AUTO_ARENA_CORE_UNIT_H
#define MY_AUTO_ARENA_CORE_UNIT_H

#include <map>
#include <string>
#include <vector>

#include "core/Item.h"

namespace my_auto_arena {
namespace core {

class Board;

enum class UnitOwner { player, enemy };

// 职业分类：用于羁绊系统判断单位类型。
enum class UnitClass { kNone, kWarrior, kArcher, kTank, kMage, kHealer };

// 单位战斗状态机，对应规格书第 3.1 节中的 S = {空闲, 移动, 攻击, 施法, 死亡}。
enum class UnitState { kIdle, kMoving, kAttacking, kCasting, kDead };

class Unit {
public:
    // 战斗单位基类：管理通用属性与基础战斗行为。
    Unit(int id, std::string name, UnitOwner owner, int maxHp, int attack, int attackRange, int maxMana,
         UnitClass unitClass = UnitClass::kNone);
    // 按课程要求显式定义拷贝构造函数，便于讲解对象复制语义。
    Unit(const Unit& other);
    // 按拷贝语义规范（拷贝构造、拷贝赋值、析构配套）要求定义拷贝赋值运算符。
    Unit& operator=(const Unit& other);
    virtual ~Unit() = default;

    int id() const;
    const std::string& name() const;
    UnitOwner owner() const;
    int hp() const;
    // 现有属性（基础 + 装备百分比加成 + 羁绊加成，四舍五入后的整数）。
    int maxHp() const;
    // attack() 保留作向后兼容，等价于 physicalAtk()。
    int attack() const;
    int physicalAtk() const;
    int magicAtk() const;
    int physicalDef() const;
    int magicDef() const;
    int attackSpeed() const;
    int attackRange() const;
    int mana() const;
    int maxMana() const;

    // 基础属性（升星直接修改这些字段；装备百分比仅基于此计算）。
    int basePhysicalAtk() const;
    int baseMagicAtk() const;
    int baseMaxHp() const;
    int basePhysicalDef() const;
    int baseMagicDef() const;
    int baseMaxMana() const;
    int baseAttackSpeed() const;
    bool isAlive() const;

    // 眩晕：剩余无法行动的 tick 数（战斗引擎每 tick 递减）。
    bool isStunned() const;
    int stunTicksRemaining() const;
    void applyStun(int ticks);
    void tickStun();

    // 第三阶段新增：职业、星级、装备访问接口。
    UnitClass unitClass() const;
    int starLevel() const;
    const std::vector<ItemType>& equippedItems() const;
    ItemType equippedItem() const;
    int equipSlotCount() const;

    // 战斗状态：由 BattleEngine 在每 tick 中更新，用于 GUI 状态展示与验收。
    UnitState state() const;
    void setState(UnitState s);

    void takeDamage(int amount);
    // 物理伤害：净伤害 = max(1, 原始伤害 - 有效 physicalDef())，保底 1 点。
    void takePhysicalDamage(int rawDmg);
    void takePhysicalDamage(int rawDmg, int defenseIgnorePercent);
    // 法术伤害：净伤害 = max(1, 原始伤害 - 有效 magicDef())，保底 1 点。
    void takeMagicDamage(int rawDmg);
    void takeMagicDamage(int rawDmg, int defenseIgnorePercent);
    void gainMana(int amount);
    // heal() 上限为 maxHp()（含羁绊加成）。
    void heal(int amount);
    // resetToFull() 将 hp_ 重置为 maxHp()，蓝量归零。
    void resetToFull();

    // 装备管理：基础属性不被装备直接改写；最终属性由访问接口叠加计算。
    void equipItem(ItemType item);
    void unequipItem();
    // 卸下指定槽位装备（0 为第一个槽位）；越界时不执行。
    void unequipItemAt(int slotIndex);
    // 卸下全部装备并返回装备列表（出售/合并消耗单位时使用）。
    std::vector<ItemType> takeAllEquippedItems();

    // 羁绊 BUFF：每轮战斗开始前设置，结束后清除。
    // 羁绊加成基于基础属性计算后临时叠加，战斗结束清除。
    void setSynergyBuffs(int bonusAtk, int bonusMagAtk, int bonusMaxHp,
                         int bonusPhysicalDef, int bonusMagicDef,
                         bool armorBreak, bool magicPenetration, bool shieldField);
    void clearSynergyBuffs();
    bool hasArmorBreak() const;
    bool hasMagicPenetration() const;
    bool hasShieldField() const;
    int physicalDefenseIgnorePercent() const;
    int magicDefenseIgnorePercent() const;
    static void beginSynergyDamageTick(int tick);
    static void endSynergyDamageTick();
    static void resetSynergyShieldState();

    // 升星：直接提升基础属性（★2=×3.0，★3=×7.0），装备加成由访问接口统一叠加。
    void upgradeToStar(int newStarLevel);

    // 按星级缩放技能基础伤害/治疗量：★1=×1.0，★2=×3.0，★3=×7.0（与物攻倍率一致）。
    int scaledSkillDamage(int baseDamage) const;

    // 法力满时由战斗引擎调用：多态技能入口；默认仅清空法力。
    virtual void castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget);

    // 普攻伤害类型标记：返回 true 表示使用法术通道（magicAtk + magicDef），
    // 返回 false 表示使用物理通道（physicalAtk + physicalDef）。
    // 由 PhysicalAttackUnit / MagicalAttackUnit 中间层重写，战斗引擎据此分发伤害类型。
    virtual bool usesMagicAttack() const;

protected:
    void spendAllMana();
    // 范围内对单目标造成普攻伤害；WarriorUnit/MageUnit 的默认技能共用此逻辑。
    void performAttackInRange(Board& board, Unit* primaryTarget);
    // 由子类构造函数调用，设置本职业特有的基础属性（如法师的 baseMagicAtk_）。
    void setBaseMagicAtk(int v);
    void setBasePhysicalDef(int v);
    void setBaseMagicDef(int v);
    // 装备法术攻击加成（仅基于基础法攻的百分比，不含羁绊）。
    int equipmentMagicAtkBonus() const;

private:
    int id_;
    std::string name_;
    UnitOwner owner_;
    int hp_;
    int baseMaxHp_;   // 不含装备/羁绊的基础最大生命值
    int baseAttack_;  // 不含装备/羁绊的基础攻击力
    int attackRange_;
    int mana_;
    int maxMana_;

    UnitClass unitClass_;
    int starLevel_;
    std::vector<ItemType> equippedItems_;
    int bonusAtk_;           // 羁绊系统临时物理攻击加成
    int bonusMagAtk_;        // 羁绊系统临时法术攻击加成（法师羁绊专用）
    int bonusMaxHp_;         // 羁绊系统临时血量加成
    int bonusPhysicalDef_;   // 羁绊系统临时物理防御加成
    int bonusMagicDef_;      // 羁绊系统临时法术防御加成
    bool armorBreak_;        // 造成物理伤害时忽略目标部分防御
    bool magicPenetration_;  // 造成法术伤害时忽略目标魔防
    bool shieldField_;       // 防御羁绊满层：受击计数并触发全队免伤
    int star1Atk_;           // 原始星级1物理攻击（升星乘算用）
    int star1MaxHp_;         // 原始星级1血量（升星乘算用）
    int star1MagAtk_;        // 原始星级1法术攻击（升星乘算用，非法师默认0）
    int baseMagicAtk_;       // 基础法术攻击（法师专属，其余默认0）
    int basePhysicalDef_;    // 基础物理防御（按职业特性设置，升星时缩放）
    int baseMagicDef_;       // 基础法术防御（按职业特性设置，升星时缩放）
    int star1PhysDef_;       // 原始星级1物理防御（升星乘算用）
    int star1MagDef_;        // 原始星级1法术防御（升星乘算用）
    UnitState state_;        // 战斗状态机当前状态，每 tick 由 BattleEngine 更新
    int stunTicksRemaining_; // 眩晕剩余 tick（>0 时本 tick 无法普攻/施法/移动）

    // 现有属性缓存（recalculateCurrentStats 更新，GUI 与战斗读取）。
    int currentPhysicalAtk_;
    int currentMagicAtk_;
    int currentMaxHp_;
    int currentPhysicalDef_;
    int currentMagicDef_;
    int currentMaxMana_;
    int currentAttackSpeed_;

    static int roundStat(double value);
    int equipmentBonusPhysAtk() const;
    int equipmentBonusMagAtk() const;
    int equipmentBonusPhysDef() const;
    int equipmentBonusMagDef() const;
    int equipmentBonusMaxHp() const;
    int equipmentBonusAttackSpeed() const;
    int equipmentBonusMaxManaFlat() const;
    void recalculateCurrentStats();
    void clampHpToCurrentMax();
    void clampManaToCurrentMax();
};

// ─────────────────────────────────────────────────────────────────────────────
// 攻击类型中间层（模板类）：
//   PhysicalAttackUnit — 物理攻击单位模板（战士/射手/重甲战士继承此层）
//   MagicalAttackUnit  — 法术攻击单位模板（法师/治疗师继承此层）
// 子类通过继承自动获得正确的普攻伤害类型，无需在战斗引擎中做 if-else 判断。
// ─────────────────────────────────────────────────────────────────────────────

// 物理攻击模板：usesMagicAttack() 返回 false，普攻走物理伤害通道。
class PhysicalAttackUnit : public Unit {
public:
    PhysicalAttackUnit(int id, const std::string& name, UnitOwner owner,
                       int maxHp, int attack, int attackRange, int maxMana,
                       UnitClass unitClass);
    // 显式拷贝构造函数：委托给 Unit，演示派生类拷贝语义。
    PhysicalAttackUnit(const PhysicalAttackUnit& other);
    virtual ~PhysicalAttackUnit() override = default;

    virtual bool usesMagicAttack() const override;
};

// 法术攻击模板：usesMagicAttack() 返回 true，普攻走法术伤害通道。
// 构造时自动调用 setBaseMagicAtk(attack)，将 attack 参数注入 baseMagicAtk_。
class MagicalAttackUnit : public Unit {
public:
    MagicalAttackUnit(int id, const std::string& name, UnitOwner owner,
                      int maxHp, int attack, int attackRange, int maxMana,
                      UnitClass unitClass);
    // 显式拷贝构造函数：委托给 Unit，baseMagicAtk_ 随 Unit 成员一并复制。
    MagicalAttackUnit(const MagicalAttackUnit& other);
    virtual ~MagicalAttackUnit() override = default;

    virtual bool usesMagicAttack() const override;
};

class WarriorUnit final : public PhysicalAttackUnit {
public:
    WarriorUnit(int id, UnitOwner owner);
    // 显式拷贝构造函数：委托给 PhysicalAttackUnit，演示继承体系中的拷贝语义。
    WarriorUnit(const WarriorUnit& other);
    virtual ~WarriorUnit() override = default;

    virtual void castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) override;
};

class MageUnit final : public MagicalAttackUnit {
public:
    MageUnit(int id, UnitOwner owner);
    // 显式拷贝构造函数：委托给 MagicalAttackUnit，演示继承体系中的拷贝语义。
    MageUnit(const MageUnit& other);
    virtual ~MageUnit() override = default;

    virtual void castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) override;
};

}  // namespace core
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_CORE_UNIT_H
