#include <gtest/gtest.h>

#include "core/HeroUnits.h"
#include "core/Item.h"
#include "core/Shop.h"
#include "core/Unit.h"

using namespace my_auto_arena::core;

// ── 商店初始化测试 ────────────────────────────────────────────────────────────

TEST(ShopTest, InitialSlotsNotSold) {
    Shop shop;
    for (int i = 0; i < Shop::kSlotCount; ++i) {
        EXPECT_FALSE(shop.slotAt(i).sold);
    }
}

TEST(ShopTest, SlotAtOutOfRangeThrows) {
    Shop shop;
    EXPECT_THROW(shop.slotAt(-1), std::out_of_range);
    EXPECT_THROW(shop.slotAt(Shop::kSlotCount), std::out_of_range);
}

// ── 购买测试 ──────────────────────────────────────────────────────────────────

TEST(ShopTest, BuyDeductsGold) {
    Shop shop;
    int gold = 10;
    Unit* hero = shop.buy(0, gold, 200);
    ASSERT_NE(hero, nullptr);
    EXPECT_EQ(gold, 10 - Shop::kHeroCost);
    delete hero;
}

TEST(ShopTest, BuyMarksSoldSlot) {
    Shop shop;
    int gold = 10;
    Unit* hero = shop.buy(0, gold, 200);
    ASSERT_NE(hero, nullptr);
    EXPECT_TRUE(shop.slotAt(0).sold);
    delete hero;
}

TEST(ShopTest, CannotBuyAlreadySoldSlot) {
    Shop shop;
    int gold = 20;
    Unit* hero1 = shop.buy(0, gold, 200);
    ASSERT_NE(hero1, nullptr);
    Unit* hero2 = shop.buy(0, gold, 201);
    EXPECT_EQ(hero2, nullptr);
    delete hero1;
}

TEST(ShopTest, CannotBuyWithInsufficientGold) {
    Shop shop;
    int gold = 1;  // 不足 kHeroCost=5
    EXPECT_FALSE(shop.canBuy(0, gold));
    Unit* hero = shop.buy(0, gold, 200);
    EXPECT_EQ(hero, nullptr);
    EXPECT_EQ(gold, 1);  // 金币未变
}

TEST(ShopTest, BuyCreatesHeroWithPlayerOwner) {
    Shop shop;
    int gold = 10;
    Unit* hero = shop.buy(0, gold, 200);
    ASSERT_NE(hero, nullptr);
    EXPECT_EQ(hero->owner(), UnitOwner::player);
    EXPECT_EQ(hero->id(), 200);
    delete hero;
}

// ── 刷新测试 ──────────────────────────────────────────────────────────────────

TEST(ShopTest, RefreshDeductsGold) {
    Shop shop;
    int gold = 10;
    shop.refresh(gold);
    EXPECT_EQ(gold, 10 - Shop::kRefreshCost);
}

TEST(ShopTest, RefreshResetsAllSoldSlots) {
    Shop shop;
    int gold = 20;
    // 购买前两个槽位
    Unit* h1 = shop.buy(0, gold, 200);
    Unit* h2 = shop.buy(1, gold, 201);
    shop.refresh(gold);
    // 刷新后所有槽位重置为未售出
    for (int i = 0; i < Shop::kSlotCount; ++i) {
        EXPECT_FALSE(shop.slotAt(i).sold);
    }
    delete h1;
    delete h2;
}

TEST(ShopTest, RefreshDoesNothingIfGoldInsufficient) {
    Shop shop;
    int gold = 1;  // 不足 kRefreshCost=2
    // 先购买一个槽位以便验证状态
    int buyGold = 10;
    Unit* h = shop.buy(0, buyGold, 200);
    // 用原始 sold 状态作为参照，刷新不应发生
    const bool soldBefore = shop.slotAt(0).sold;
    shop.refresh(gold);
    EXPECT_EQ(gold, 1);  // 金币未变
    EXPECT_EQ(shop.slotAt(0).sold, soldBefore);  // 状态未变
    delete h;
}

// ── 出售价值测试 ──────────────────────────────────────────────────────────────

TEST(ShopTest, SellValueStar1Is1) {
    EXPECT_EQ(Shop::sellValue(1), 1);
}

TEST(ShopTest, SellValueStar2Is2) {
    EXPECT_EQ(Shop::sellValue(2), 2);
}

TEST(ShopTest, SellValueStar3Is4) {
    EXPECT_EQ(Shop::sellValue(3), 4);
}

// ── 拷贝构造测试 ──────────────────────────────────────────────────────────────

TEST(ShopTest, CopyConstructorCopiesSlots) {
    Shop shop;
    Shop copy(shop);
    for (int i = 0; i < Shop::kSlotCount; ++i) {
        EXPECT_EQ(shop.slotAt(i).heroType, copy.slotAt(i).heroType);
        EXPECT_EQ(shop.slotAt(i).sold, copy.slotAt(i).sold);
    }
}
