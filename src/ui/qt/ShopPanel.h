#ifndef MY_AUTO_ARENA_UI_QT_SHOP_PANEL_H
#define MY_AUTO_ARENA_UI_QT_SHOP_PANEL_H

#include <QPushButton>
#include <QWidget>

#include "core/Shop.h"

namespace my_auto_arena {
namespace ui {

// 商店面板：显示 5 个英雄购买槽、刷新按钮和当前金币。
class ShopPanel : public QWidget {
    Q_OBJECT
public:
    explicit ShopPanel(QWidget* parent = nullptr);

    // 根据当前商店状态和金币数量更新所有槽位按钮。
    void updateDisplay(const core::Shop& shop, int playerGold);

signals:
    void heroPurchased(int slotIndex);
    void refreshRequested();

private slots:
    void onSlotClicked();
    void onRefreshClicked();

private:
    QPushButton* slotBtns_[core::Shop::kSlotCount];
    QPushButton* refreshBtn_;

    // 返回英雄类型对应的中文显示名称。
    static QString heroTypeName(core::HeroType type);
};

}  // namespace ui
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_UI_QT_SHOP_PANEL_H
