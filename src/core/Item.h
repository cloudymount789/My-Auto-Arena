#ifndef MY_AUTO_ARENA_CORE_ITEM_H
#define MY_AUTO_ARENA_CORE_ITEM_H

#include <string>

namespace my_auto_arena {
namespace core {

// 装备类型枚举：玩家可收集并装备到英雄身上的道具。
enum class ItemType { kNone, kSword, kArmor, kRing, kTalisman };

// 装备属性定义：名称、攻击加成、最大生命值加成。
struct ItemDef {
    std::string name;
    int bonusAtk;
    int bonusMaxHp;
};

// 根据装备类型返回对应的属性定义。
const ItemDef& getItemDef(ItemType type);

}  // namespace core
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_CORE_ITEM_H
