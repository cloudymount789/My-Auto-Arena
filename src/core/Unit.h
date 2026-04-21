#ifndef MY_AUTO_ARENA_CORE_UNIT_H
#define MY_AUTO_ARENA_CORE_UNIT_H

#include <map>
#include <string>

namespace my_auto_arena {
namespace core {

class Board;

enum class UnitOwner { player, enemy };

class Unit {
public:
    // 战斗单位基类：管理通用属性与基础战斗行为。
    Unit(int id, std::string name, UnitOwner owner, int maxHp, int attack, int attackRange, int maxMana);
    // 按课程要求显式定义拷贝构造函数，便于讲解对象复制语义。
    Unit(const Unit& other);
    virtual ~Unit() = default;

    int id() const;
    const std::string& name() const;
    UnitOwner owner() const;
    int hp() const;
    int maxHp() const;
    int attack() const;
    int attackRange() const;
    int mana() const;
    int maxMana() const;
    bool isAlive() const;

    void takeDamage(int amount);
    void gainMana(int amount);
    void heal(int amount);

    // 法力满时由战斗引擎调用：多态技能入口；默认仅清空法力。
    virtual void castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget);

protected:
    void spendAllMana();

private:
    int id_;
    std::string name_;
    UnitOwner owner_;
    int hp_;
    int maxHp_;
    int attack_;
    int attackRange_;
    int mana_;
    int maxMana_;
};

class WarriorUnit final : public Unit {
public:
    WarriorUnit(int id, UnitOwner owner);
    virtual ~WarriorUnit() override = default;

    void castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) override;
};

class MageUnit final : public Unit {
public:
    MageUnit(int id, UnitOwner owner);
    virtual ~MageUnit() override = default;

    void castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) override;
};

}  // namespace core
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_CORE_UNIT_H
