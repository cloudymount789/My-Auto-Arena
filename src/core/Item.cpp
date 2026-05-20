#include "core/Item.h"

#include <stdexcept>

namespace my_auto_arena {
namespace core {

const ItemDef& getItemDef(ItemType type) {
    // 静态装备定义表：铁剑/锁甲/魔纹环/疗愈符，按需索引。
    static const ItemDef kNoneDef    = {"无",    0,   0};
    static const ItemDef kSwordDef   = {"铁剑",  80,  0};
    static const ItemDef kArmorDef   = {"锁甲",  0,   800};
    static const ItemDef kRingDef    = {"魔纹环", 60,  0};
    static const ItemDef kTalismanDef= {"疗愈符", 0,   600};

    switch (type) {
        case ItemType::kNone:     return kNoneDef;
        case ItemType::kSword:    return kSwordDef;
        case ItemType::kArmor:    return kArmorDef;
        case ItemType::kRing:     return kRingDef;
        case ItemType::kTalisman: return kTalismanDef;
        default:
            throw std::invalid_argument("未知装备类型");
    }
}

}  // namespace core
}  // namespace my_auto_arena
