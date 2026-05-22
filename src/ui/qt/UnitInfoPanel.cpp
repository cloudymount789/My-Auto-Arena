#include "ui/qt/UnitInfoPanel.h"

#include <QVBoxLayout>

#include "core/Item.h"

namespace my_auto_arena {
namespace ui {

UnitInfoPanel::UnitInfoPanel(QWidget* parent) : QWidget(parent), currentUnitId_(-1) {
    setStyleSheet("background-color: #1E1E2E; border-radius: 6px; color: #CDD6F4;");
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(4);

    name_ = new QLabel("名称: -", this);
    name_->setStyleSheet("font-weight: bold; font-size: 13px; color: #89DCEB;");

    classLabel_ = new QLabel("职业: -", this);
    classLabel_->setStyleSheet("font-size: 12px; color: #A6E3A1;");

    starLabel_ = new QLabel("星级: -", this);
    starLabel_->setStyleSheet("font-size: 12px; color: #FFD700;");

    itemLabel_ = new QLabel("装备: 无", this);
    itemLabel_->setStyleSheet("font-size: 12px; color: #CBA6F7;");

    attack_ = new QLabel("ATK: -", this);
    range_  = new QLabel("射程: -", this);

    hp_   = new QProgressBar(this);
    mana_ = new QProgressBar(this);
    hp_->setFormat("HP %v/%m");
    hp_->setStyleSheet("QProgressBar::chunk { background-color: #A6E3A1; }");
    mana_->setFormat("Mana %v/%m");
    mana_->setStyleSheet("QProgressBar::chunk { background-color: #89DCEB; }");

    sellBtn_ = new QPushButton("出售", this);
    sellBtn_->setEnabled(false);
    sellBtn_->setStyleSheet(
        "QPushButton { background-color: #F38BA8; color: #1E1E2E; font-weight: bold;"
        " border-radius: 4px; padding: 4px; }"
        "QPushButton:disabled { background-color: #45475A; color: #6C7086; }");
    connect(sellBtn_, SIGNAL(clicked()), this, SLOT(onSellClicked()));

    unequipBtn_ = new QPushButton("卸下装备", this);
    unequipBtn_->setEnabled(false);
    unequipBtn_->setStyleSheet(
        "QPushButton { background-color: #CBA6F7; color: #1E1E2E; font-weight: bold;"
        " border-radius: 4px; padding: 4px; }"
        "QPushButton:disabled { background-color: #45475A; color: #6C7086; }");
    connect(unequipBtn_, SIGNAL(clicked()), this, SLOT(onUnequipClicked()));

    layout->addWidget(name_);
    layout->addWidget(classLabel_);
    layout->addWidget(starLabel_);
    layout->addWidget(itemLabel_);
    layout->addWidget(attack_);
    layout->addWidget(range_);
    layout->addWidget(hp_);
    layout->addWidget(mana_);
    layout->addWidget(unequipBtn_);
    layout->addWidget(sellBtn_);
    layout->addStretch();
}

void UnitInfoPanel::setUnit(const core::Unit* unit) {
    if (unit == nullptr) {
        name_->setText("名称: -");
        classLabel_->setText("职业: -");
        starLabel_->setText("星级: -");
        itemLabel_->setText("装备: 无");
        attack_->setText("ATK: -");
        range_->setText("射程: -");
        hp_->setMaximum(1);
        hp_->setValue(0);
        mana_->setMaximum(1);
        mana_->setValue(0);
        sellBtn_->setEnabled(false);
        unequipBtn_->setEnabled(false);
        currentUnitId_ = -1;
        return;
    }

    currentUnitId_ = unit->id();
    name_->setText(QString("名称: %1").arg(QString::fromStdString(unit->name())));
    classLabel_->setText(QString("职业: %1").arg(unitClassName(unit->unitClass())));

    // 星级显示为 ★ 符号序列。
    QString stars;
    for (int i = 0; i < unit->starLevel(); ++i) {
        stars += QString::fromUtf8("\xe2\x98\x85");
    }
    starLabel_->setText(QString("星级: %1").arg(stars));

    const std::string& itemName = core::getItemDef(unit->equippedItem()).name;
    itemLabel_->setText(QString("装备: %1").arg(QString::fromStdString(itemName)));

    attack_->setText(QString("ATK: %1").arg(unit->attack()));
    range_->setText(QString("射程: %1").arg(unit->attackRange()));
    hp_->setMaximum(unit->maxHp());
    hp_->setValue(unit->hp());
    mana_->setMaximum(unit->maxMana());
    mana_->setValue(unit->mana());

    const bool isPlayerUnit = (unit->owner() == core::UnitOwner::player);
    sellBtn_->setEnabled(isPlayerUnit);
    // 仅玩家单位且已装备道具时可卸装。
    unequipBtn_->setEnabled(isPlayerUnit && unit->equippedItem() != core::ItemType::kNone);
}

void UnitInfoPanel::onSellClicked() {
    if (currentUnitId_ >= 0) {
        emit sellRequested(currentUnitId_);
    }
}

void UnitInfoPanel::onUnequipClicked() {
    if (currentUnitId_ >= 0) {
        emit unequipRequested(currentUnitId_);
    }
}

QString UnitInfoPanel::unitClassName(core::UnitClass cls) {
    switch (cls) {
        case core::UnitClass::kWarrior: return "战士";
        case core::UnitClass::kArcher:  return "射手";
        case core::UnitClass::kTank:    return "重甲战士";
        case core::UnitClass::kMage:    return "法师";
        case core::UnitClass::kHealer:  return "治疗师";
        default:                        return "通用";
    }
}

}  // namespace ui
}  // namespace my_auto_arena
