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

const ShopSlot& Shop::slotAt(int index) const {
    if (index < 0 || index >= kSlotCount) {
        throw std::out_of_range("商店槽位索引越界");
    }
    return slots_[index];
}

void Shop::refresh(int& playerGold) {
    if (playerGold < kRefreshCost) {
        return;
    }
    playerGold -= kRefreshCost;
    randomizeSlots();
}

bool Shop::canBuy(int slotIndex, int playerGold) const {
    if (slotIndex < 0 || slotIndex >= kSlotCount) {
        return false;
    }
    return !slots_[slotIndex].sold && playerGold >= kHeroCost;
}

Unit* Shop::buy(int slotIndex, int& playerGold, int newUnitId) {
    if (!canBuy(slotIndex, playerGold)) {
        return nullptr;
    }
    slots_[slotIndex].sold = true;
    playerGold -= kHeroCost;
    return createHero(slots_[slotIndex].heroType, newUnitId, UnitOwner::player);
}

int Shop::sellValue(int starLevel) {
    // star1=1金，star2=2金，star3=4金。
    if (starLevel == 2) return 2;
    if (starLevel >= 3) return 4;
    return 1;
}

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
