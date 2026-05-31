#ifndef MY_AUTO_ARENA_CORE_HERO_UNITS_H
#define MY_AUTO_ARENA_CORE_HERO_UNITS_H

#include <map>

#include "core/Board.h"
#include "core/Unit.h"

namespace my_auto_arena {
namespace core {

// 五种带技能的英雄单位（满蓝施法），用于第二阶段技能多态演示。
// 继承自 PhysicalAttackUnit 或 MagicalAttackUnit，普攻伤害类型由中间层决定，
// 无需在战斗引擎中对职业做逐一判断。

// 战士：继承物理攻击模板，普攻与技能均为物理伤害。
class AshRaiderHero final : public PhysicalAttackUnit {
public:
    AshRaiderHero(int id, UnitOwner owner);
    // 显式拷贝构造函数：委托给 PhysicalAttackUnit，便于课程讲解派生类拷贝语义。
    AshRaiderHero(const AshRaiderHero& other);
    virtual void castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) override;
    virtual ~AshRaiderHero() override = default;
};

// 射手：继承物理攻击模板，远程物理输出（射程4）。
class NightArcherHero final : public PhysicalAttackUnit {
public:
    NightArcherHero(int id, UnitOwner owner);
    NightArcherHero(const NightArcherHero& other);
    virtual void castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) override;
    virtual ~NightArcherHero() override = default;
};

// 重甲战士：继承物理攻击模板，范围伤害技能。
class CurseHammerHero final : public PhysicalAttackUnit {
public:
    CurseHammerHero(int id, UnitOwner owner);
    CurseHammerHero(const CurseHammerHero& other);
    virtual void castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) override;
    virtual ~CurseHammerHero() override = default;
};

// 法师：继承法术攻击模板，普攻与技能均为法术伤害。
// MagicalAttackUnit 构造时自动 setBaseMagicAtk(attack)，无需手动调用。
class MistWitchHero final : public MagicalAttackUnit {
public:
    MistWitchHero(int id, UnitOwner owner);
    MistWitchHero(const MistWitchHero& other);
    virtual void castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) override;
    virtual ~MistWitchHero() override = default;
};

// 治疗师：继承法术攻击模板，普攻也走法术通道，技能为范围治疗。
class BonePrayerHero final : public MagicalAttackUnit {
public:
    BonePrayerHero(int id, UnitOwner owner);
    BonePrayerHero(const BonePrayerHero& other);
    virtual void castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) override;
    virtual ~BonePrayerHero() override = default;
};

// 英雄类型枚举：商店购买与工厂创建使用。
enum class HeroType { kWarrior, kArcher, kTank, kMage, kHealer };

// 英雄工厂函数：根据类型创建对应英雄实例（堆分配，调用方负责释放）。
Unit* createHero(HeroType type, int id, UnitOwner owner);

}  // namespace core
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_CORE_HERO_UNITS_H
