#ifndef MY_AUTO_ARENA_CORE_ITEM_H
#define MY_AUTO_ARENA_CORE_ITEM_H

#include <string>

namespace my_auto_arena {
namespace core {

// 装备类型枚举：玩家可收集并装备到英雄身上的道具。
enum class ItemType {
    kNone,
    kSword,
    kArmor,
    kRing,
    kTalisman,
    kRunicShield,
    kSwiftGloves,
    kBlueCrystal
};

// 装备属性定义：百分比加成仅作用于单位「基础属性」，羁绊加成为战斗时额外叠加。
struct ItemDef {
    std::string name;
    int bonusPhysAtkPercent;       // 物理攻击 +N%（相对基础物攻）
    int bonusMagAtkPercent;        // 法术攻击 +N%（相对基础法攻）
    int bonusPhysDefensePercent;   // 物理防御 +N%（相对基础物防）
    int bonusMagDefensePercent;    // 法术防御 +N%（相对基础魔防）
    int bonusMaxHpPercent;         // 最大生命 +N%（相对基础生命）
    int bonusAttackSpeedPercent;   // 攻击速度 +N%（相对基础攻速 100）
    int bonusMaxManaFlat;          // 最大法力固定修正（蓝水晶为 -30）
};

// 根据装备类型返回对应的属性定义。
const ItemDef& getItemDef(ItemType type);

}  // namespace core
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_CORE_ITEM_H
