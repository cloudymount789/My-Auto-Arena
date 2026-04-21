#ifndef MY_AUTO_ARENA_CORE_HERO_UNITS_H
#define MY_AUTO_ARENA_CORE_HERO_UNITS_H

#include <map>

#include "core/Board.h"
#include "core/Unit.h"

namespace my_auto_arena {
namespace core {

// 五种带技能的英雄单位（满蓝施法），用于 Phase2 技能多态演示。

class AshRaiderHero final : public Unit {
public:
    AshRaiderHero(int id, UnitOwner owner);
    virtual void castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) override;
    virtual ~AshRaiderHero() override = default;
};

class NightArcherHero final : public Unit {
public:
    NightArcherHero(int id, UnitOwner owner);
    virtual void castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) override;
    virtual ~NightArcherHero() override = default;
};

class CurseHammerHero final : public Unit {
public:
    CurseHammerHero(int id, UnitOwner owner);
    virtual void castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) override;
    virtual ~CurseHammerHero() override = default;
};

class MistWitchHero final : public Unit {
public:
    MistWitchHero(int id, UnitOwner owner);
    virtual void castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) override;
    virtual ~MistWitchHero() override = default;
};

class BonePrayerHero final : public Unit {
public:
    BonePrayerHero(int id, UnitOwner owner);
    virtual void castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) override;
    virtual ~BonePrayerHero() override = default;
};

}  // namespace core
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_CORE_HERO_UNITS_H
