#include "core/Item.h"

#include <stdexcept>

namespace my_auto_arena {
namespace core {

// 流程：懒初始化静态 ItemDef 表 ──> switch 装备类型 ──> 返回对应定义引用（未知类型抛异常）
const ItemDef& getItemDef(ItemType type) {
    // 百分比加成仅作用于单位「基础属性」；蓝水晶为最大法力固定修正。
    static const ItemDef kNoneDef         = {"无",       0,  0,  0,  0,   0,  0,   0};
    static const ItemDef kSwordDef        = {"铁剑",    15,  0,  0,  0,   0,  0,   0};
    static const ItemDef kArmorDef        = {"锁甲",     0,  0, 25,  0,   0,  0,   0};
    static const ItemDef kRingDef         = {"魔纹环",   0, 15,  0,  0,   0,  0,   0};
    static const ItemDef kTalismanDef     = {"疗愈符",   0,  0,  0,  0,  20,  0,   0};
    static const ItemDef kRunicShieldDef  = {"符文盾",   0,  0,  0, 20,   0,  0,   0};
    static const ItemDef kSwiftGlovesDef  = {"急速手套", 0,  0,  0,  0,   0, 15,   0};
    static const ItemDef kBlueCrystalDef  = {"蓝水晶",   0,  0,  0,  0,   0,  0, -30};

    switch (type) {
        case ItemType::kNone:        return kNoneDef;
        case ItemType::kSword:       return kSwordDef;
        case ItemType::kArmor:       return kArmorDef;
        case ItemType::kRing:        return kRingDef;
        case ItemType::kTalisman:    return kTalismanDef;
        case ItemType::kRunicShield: return kRunicShieldDef;
        case ItemType::kSwiftGloves: return kSwiftGlovesDef;
        case ItemType::kBlueCrystal: return kBlueCrystalDef;
        default:
            throw std::invalid_argument("未知装备类型");
    }
}

}  // namespace core
}  // namespace my_auto_arena
