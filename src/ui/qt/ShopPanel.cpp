#include "ui/qt/ShopPanel.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace my_auto_arena {
namespace ui {

// 流程：搭建标题与槽位按钮行 ──> 绑定点击信号 ──> 添加刷新按钮
ShopPanel::ShopPanel(QWidget* parent) : QWidget(parent) {
    setStyleSheet("background-color: #181825; border-radius: 6px;");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(6, 6, 6, 6);
    mainLayout->setSpacing(6);

    // 标题行
    QLabel* title = new QLabel("商店", this);
    title->setStyleSheet("color: #F9E2AF; font-weight: bold; font-size: 13px;");
    mainLayout->addWidget(title);

    // 英雄槽位按钮行
    QWidget* slotsRow = new QWidget(this);
    QHBoxLayout* slotsLayout = new QHBoxLayout(slotsRow);
    slotsLayout->setContentsMargins(0, 0, 0, 0);
    slotsLayout->setSpacing(4);

    const QString slotStyle =
        "QPushButton { background-color: #313244; color: #CDD6F4; font-size: 11px;"
        " border-radius: 4px; padding: 4px; min-width: 60px; min-height: 50px; }"
        "QPushButton:disabled { background-color: #45475A; color: #6C7086; }";

    for (int i = 0; i < core::Shop::kSlotCount; ++i) {
        slotBtns_[i] = new QPushButton(this);
        slotBtns_[i]->setStyleSheet(slotStyle);
        slotsLayout->addWidget(slotBtns_[i]);

        // 绑定点击信号，通过类似 lambda 的方式传递槽位索引（Qt4 风格：手动映射）。
        slotBtns_[i]->setProperty("slotIndex", i);
        connect(slotBtns_[i], SIGNAL(clicked()), this, SLOT(onSlotClicked()));
    }
    // 由于 Qt4 风格 SIGNAL/SLOT 不支持带参数的直接绑定，可改用 QSignalMapper 替代方案。
    // 这里使用简单的 sender() 判断点击来源。
    mainLayout->addWidget(slotsRow);

    // 刷新按钮
    refreshBtn_ = new QPushButton(QString("刷新 -%1金").arg(core::Shop::kRefreshCost), this);
    refreshBtn_->setStyleSheet(
        "QPushButton { background-color: #89DCEB; color: #1E1E2E; font-weight: bold;"
        " border-radius: 4px; padding: 4px 10px; }"
        "QPushButton:disabled { background-color: #45475A; color: #6C7086; }");
    connect(refreshBtn_, SIGNAL(clicked()), this, SLOT(onRefreshClicked()));
    mainLayout->addWidget(refreshBtn_);
}

// 流程：遍历 5 个槽位 ──> 已售出则禁用 ──> 否则显示英雄名与价格并校验金币
void ShopPanel::updateDisplay(const core::Shop& shop, int playerGold) {
    for (int i = 0; i < core::Shop::kSlotCount; ++i) {
        const core::ShopSlot& slot = shop.slotAt(i);
        if (slot.sold) {
            slotBtns_[i]->setText("已售出");
            slotBtns_[i]->setEnabled(false);
        } else {
            slotBtns_[i]->setText(
                heroTypeName(slot.heroType) + "\n" +
                QString("%1金").arg(core::Shop::kHeroCost));
            slotBtns_[i]->setEnabled(playerGold >= core::Shop::kHeroCost);
        }
    }
}

// 流程：从 sender 取被点击按钮 ──> 读取槽位索引属性 ──> 发出购买请求信号
void ShopPanel::onSlotClicked() {
    // 通过 sender() 获取点击的槽位按钮，读取其 slotIndex 属性。
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    if (btn == nullptr) return;
    const int idx = btn->property("slotIndex").toInt();
    emit heroPurchased(idx);
}

void ShopPanel::onRefreshClicked() {
    emit refreshRequested();
}

// 流程：switch HeroType ──> 返回中文职业名
QString ShopPanel::heroTypeName(core::HeroType type) {
    switch (type) {
        case core::HeroType::kWarrior: return "战士";
        case core::HeroType::kArcher:  return "射手";
        case core::HeroType::kTank:    return "重甲";
        case core::HeroType::kMage:    return "法师";
        case core::HeroType::kHealer:  return "治疗师";
        default:                       return "未知";
    }
}

}  // namespace ui
}  // namespace my_auto_arena
