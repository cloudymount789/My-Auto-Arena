#ifndef MY_AUTO_ARENA_CORE_SHOP_H
#define MY_AUTO_ARENA_CORE_SHOP_H

#include "core/HeroUnits.h"
#include "core/Unit.h"

namespace my_auto_arena {
namespace core {

// 商店槽位：记录英雄类型与售出状态。
struct ShopSlot {
    HeroType heroType;
    bool sold;
};

// 商店系统：提供 5 个随机英雄购买槽，支持刷新和出售。
class Shop {
public:
    static const int kSlotCount   = 5;
    static const int kRefreshCost = 2;
    static const int kHeroCost    = 3;

    Shop();
    Shop(const Shop& other);

    const ShopSlot& slotAt(int index) const;

    // 花费 kRefreshCost 金币随机刷新所有槽位；金币不足则不执行。
    void refresh(int& playerGold);

    // 检查指定槽位是否可购买（未售出且金币足够）。
    bool canBuy(int slotIndex, int playerGold) const;

    // 购买英雄：扣除金币，标记槽位售出，创建并返回英雄实例。
    Unit* buy(int slotIndex, int& playerGold, int newUnitId);

    // 出售英雄获得金币：star1=1，star2=2，star3=4。
    static int sellValue(int starLevel);

private:
    ShopSlot slots_[kSlotCount];

    void randomizeSlots();
};

}  // namespace core
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_CORE_SHOP_H
