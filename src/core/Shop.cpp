#include "core/Shop.h"

#include <cstdlib>
#include <stdexcept>

namespace my_auto_arena {
namespace core {

Shop::Shop() {
    randomizeSlots();
}

Shop::Shop(const Shop& other) {
    for (int i = 0; i < kSlotCount; ++i) {
        slots_[i] = other.slots_[i];
    }
}

// 流程：校验槽位下标 ──> 非法抛异常 ──> 返回指定商店槽只读引用
const ShopSlot& Shop::slotAt(int index) const {
    if (index < 0 || index >= kSlotCount) {
        throw std::out_of_range("商店槽位索引越界");
    }
    return slots_[index];
}

// 流程：检查金币是否足够 ──> 扣刷新费用 ──> 重新随机所有槽位
void Shop::refresh(int& playerGold) {
    if (playerGold < kRefreshCost) {
        return;
    }
    playerGold -= kRefreshCost;
    randomizeSlots();
}

// 流程：校验槽位范围与售出状态 ──> 检查玩家金币 ──> 返回能否购买
bool Shop::canBuy(int slotIndex, int playerGold) const {
    if (slotIndex < 0 || slotIndex >= kSlotCount) {
        return false;
    }
    return !slots_[slotIndex].sold && playerGold >= kHeroCost;
}

// 流程：校验购买条件 ──> 扣金币并标记槽位售出 ──> 创建对应英雄实例返回
Unit* Shop::buy(int slotIndex, int& playerGold, int newUnitId) {
    if (!canBuy(slotIndex, playerGold)) {
        return nullptr;
    }
    slots_[slotIndex].sold = true;
    playerGold -= kHeroCost;
    return createHero(slots_[slotIndex].heroType, newUnitId, UnitOwner::player);
}

void Shop::cancelSlotSale(int slotIndex) {
    if (slotIndex >= 0 && slotIndex < kSlotCount) {
        slots_[slotIndex].sold = false;
    }
}

// 流程：按星级映射出售价格 ──> 3星及以上按4金 ──> 其他默认1金
int Shop::sellValue(int starLevel) {
    // star1=1金，star2=2金，star3=4金。
    if (starLevel == 2) return 2;
    if (starLevel >= 3) return 4;
    return 1;
}

// 流程：从 5 种英雄类型随机抽取 ──> 填入各槽位 ──> 重置 sold 为未售出
void Shop::randomizeSlots() {
    // 随机选取 5 个英雄类型（允许重复，体现卡池机制）。
    const HeroType allTypes[5] = {
        HeroType::kWarrior, HeroType::kArcher, HeroType::kTank,
        HeroType::kMage,    HeroType::kHealer
    };
    for (int i = 0; i < kSlotCount; ++i) {
        slots_[i].heroType = allTypes[std::rand() % 5];
        slots_[i].sold = false;
    }
}

}  // namespace core
}  // namespace my_auto_arena
