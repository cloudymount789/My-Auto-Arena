#include "core/Item.h"

#include <stdexcept>

namespace my_auto_arena {
namespace core {

const ItemDef& getItemDef(ItemType type) {
    // 静态装备定义表：字段顺序 name/bonusPhysAtk/bonusMagAtk/bonusPhysDefense/bonusMagDefense/bonusMaxHp
    // 铁剑：物理输出向，战士/射手首选。
    // 锁甲：物理防御向，坦克/战士适用。
    // 魔纹环：法术攻击向，法师专属。
    // 疗愈符：最大HP向，治疗师技能收益随maxHp提升。
    // 符文盾：法术防御向，对抗法师敌人时使用。
    static const ItemDef kNoneDef         = {"无",    0,   0,  0,  0,   0};
    static const ItemDef kSwordDef        = {"铁剑",  80,  0,  0,  0,   0};
    static const ItemDef kArmorDef        = {"锁甲",  0,   0,  50, 0,   0};
    static const ItemDef kRingDef         = {"魔纹环", 0,  60,  0,  0,   0};
    static const ItemDef kTalismanDef     = {"疗愈符", 0,   0,  0,  0, 800};
    static const ItemDef kRunicShieldDef  = {"符文盾", 0,   0,  0,  50,  0};

    switch (type) {
        case ItemType::kNone:        return kNoneDef;
        case ItemType::kSword:       return kSwordDef;
        case ItemType::kArmor:       return kArmorDef;
        case ItemType::kRing:        return kRingDef;
        case ItemType::kTalisman:    return kTalismanDef;
        case ItemType::kRunicShield: return kRunicShieldDef;
        default:
            throw std::invalid_argument("未知装备类型");
    }
}

}  // namespace core
}  // namespace my_auto_arena
