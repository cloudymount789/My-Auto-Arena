#ifndef MY_AUTO_ARENA_CORE_ITEM_H
#define MY_AUTO_ARENA_CORE_ITEM_H

#include <string>

namespace my_auto_arena {
namespace core {

// 装备类型枚举：玩家可收集并装备到英雄身上的道具。
enum class ItemType { kNone, kSword, kArmor, kRing, kTalisman, kRunicShield };

// 装备属性定义：各加成字段独立，支持物理/法术两类战斗属性。
struct ItemDef {
    std::string name;
    int bonusPhysAtk;      // 物理攻击加成
    int bonusMagAtk;       // 法术攻击加成
    int bonusPhysDefense;  // 物理防御加成
    int bonusMagDefense;   // 法术防御加成
    int bonusMaxHp;        // 最大生命值加成
};

// 根据装备类型返回对应的属性定义。
const ItemDef& getItemDef(ItemType type);

}  // namespace core
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_CORE_ITEM_H
