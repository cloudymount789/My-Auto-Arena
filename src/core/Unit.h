#ifndef MY_AUTO_ARENA_CORE_UNIT_H
#define MY_AUTO_ARENA_CORE_UNIT_H

#include <string>

namespace my_auto_arena::core {

enum class UnitOwner { player, enemy };

class Unit {
public:
    Unit(int id, std::string name, UnitOwner owner, int max_hp, int attack, int attack_range, int max_mana);
    virtual ~Unit() = default;

    [[nodiscard]] int id() const noexcept;
    [[nodiscard]] const std::string& name() const noexcept;
    [[nodiscard]] UnitOwner owner() const noexcept;
    [[nodiscard]] int hp() const noexcept;
    [[nodiscard]] int max_hp() const noexcept;
    [[nodiscard]] int attack() const noexcept;
    [[nodiscard]] int attack_range() const noexcept;
    [[nodiscard]] int mana() const noexcept;
    [[nodiscard]] int max_mana() const noexcept;
    [[nodiscard]] bool is_alive() const noexcept;

    void take_damage(int amount) noexcept;
    void gain_mana(int amount) noexcept;

protected:
private:
    int id_;
    std::string name_;
    UnitOwner owner_;
    int hp_;
    int max_hp_;
    int attack_;
    int attack_range_;
    int mana_{0};
    int max_mana_;
};

class WarriorUnit final : public Unit {
public:
    explicit WarriorUnit(int id, UnitOwner owner);
};

class MageUnit final : public Unit {
public:
    explicit MageUnit(int id, UnitOwner owner);
};

}  // namespace my_auto_arena::core

#endif  // MY_AUTO_ARENA_CORE_UNIT_H
