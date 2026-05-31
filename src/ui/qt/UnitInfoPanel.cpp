#include "ui/qt/UnitInfoPanel.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

#include "core/Item.h"

namespace my_auto_arena {
namespace ui {

// 流程：创建标签/进度条/出售按钮 ──> 组装垂直布局 ──> 连接出售信号
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

    equipSlotLabel_ = new QLabel("装备槽: -", this);
    equipSlotLabel_->setStyleSheet("font-size: 12px; color: #CBA6F7;");

    equipSlotsWidget_ = new QWidget(this);
    equipSlotsLayout_ = new QVBoxLayout(equipSlotsWidget_);
    equipSlotsLayout_->setContentsMargins(0, 0, 0, 0);
    equipSlotsLayout_->setSpacing(3);

    atkLabel_      = new QLabel("ATK: -", this);
    physDefLabel_  = new QLabel("物防: -", this);
    magDefLabel_   = new QLabel("魔防: -", this);
    atkSpeedLabel_ = new QLabel("攻速: -", this);
    range_         = new QLabel("射程: -", this);

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

    layout->addWidget(name_);
    layout->addWidget(classLabel_);
    layout->addWidget(starLabel_);
    layout->addWidget(equipSlotLabel_);
    layout->addWidget(equipSlotsWidget_);
    layout->addWidget(atkLabel_);
    layout->addWidget(physDefLabel_);
    layout->addWidget(magDefLabel_);
    layout->addWidget(atkSpeedLabel_);
    layout->addWidget(range_);
    layout->addWidget(hp_);
    layout->addWidget(mana_);
    layout->addWidget(sellBtn_);
    layout->addStretch();
}

void UnitInfoPanel::clearEquipSlotRows() {
    while (equipSlotsLayout_->count() > 0) {
        QLayoutItem* item = equipSlotsLayout_->takeAt(0);
        if (item->widget() != nullptr) {
            delete item->widget();
        }
        delete item;
    }
}

// 流程：清空旧行 ──> 无装备则占位文案 ──> 逐槽创建装备名+卸下按钮
void UnitInfoPanel::rebuildEquipSlotRows(const core::Unit* unit, bool isPlayerUnit) {
    clearEquipSlotRows();

    if (unit == nullptr) {
        QLabel* empty = new QLabel("（无装备）", equipSlotsWidget_);
        empty->setStyleSheet("color: #6C7086; font-size: 11px;");
        equipSlotsLayout_->addWidget(empty);
        return;
    }

    const std::vector<core::ItemType>& items = unit->equippedItems();
    if (items.empty()) {
        QLabel* empty = new QLabel("（空槽，可从右侧待装备列表穿戴）", equipSlotsWidget_);
        empty->setStyleSheet("color: #6C7086; font-size: 11px;");
        empty->setWordWrap(true);
        equipSlotsLayout_->addWidget(empty);
        return;
    }

    for (int slot = 0; slot < static_cast<int>(items.size()); ++slot) {
        const core::ItemDef& def = core::getItemDef(items.at(static_cast<std::size_t>(slot)));
        QWidget* row = new QWidget(equipSlotsWidget_);
        QHBoxLayout* rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        rowLayout->setSpacing(6);

        QLabel* label = new QLabel(
            QString("槽%1: %2").arg(slot + 1).arg(QString::fromStdString(def.name)), row);
        label->setStyleSheet("font-size: 11px; color: #CDD6F4;");

        QPushButton* btn = new QPushButton("卸下", row);
        btn->setProperty("slotIndex", slot);
        btn->setEnabled(isPlayerUnit);
        btn->setStyleSheet(
            "QPushButton { background-color: #45475A; color: #CBA6F7; font-size: 10px;"
            " border-radius: 3px; padding: 2px 6px; }"
            "QPushButton:disabled { color: #6C7086; }");
        connect(btn, SIGNAL(clicked()), this, SLOT(onUnequipSlotClicked()));

        rowLayout->addWidget(label, 1);
        rowLayout->addWidget(btn);
        equipSlotsLayout_->addWidget(row);
    }
}

// 流程：unit 为空则重置面板 ──> 否则填充名称/职业/星级/攻防/血蓝 ──> 重建装备槽行
void UnitInfoPanel::setUnit(const core::Unit* unit) {
    if (unit == nullptr) {
        name_->setText("名称: -");
        classLabel_->setText("职业: -");
        starLabel_->setText("星级: -");
        equipSlotLabel_->setText("装备槽: -");
        clearEquipSlotRows();
        QLabel* empty = new QLabel("（无装备）", equipSlotsWidget_);
        empty->setStyleSheet("color: #6C7086; font-size: 11px;");
        equipSlotsLayout_->addWidget(empty);
        atkLabel_->setText("ATK: -");
        physDefLabel_->setText("物防: -");
        magDefLabel_->setText("魔防: -");
        atkSpeedLabel_->setText("攻速: -");
        range_->setText("射程: -");
        hp_->setMaximum(1);
        hp_->setValue(0);
        mana_->setMaximum(1);
        mana_->setValue(0);
        sellBtn_->setEnabled(false);
        currentUnitId_ = -1;
        return;
    }

    currentUnitId_ = unit->id();
    name_->setText(QString("名称: %1").arg(QString::fromStdString(unit->name())));
    classLabel_->setText(QString("职业: %1").arg(unitClassName(unit->unitClass())));

    QString stars;
    for (int i = 0; i < unit->starLevel(); ++i) {
        stars += QString::fromUtf8("\xe2\x98\x85");
    }
    starLabel_->setText(QString("星级: %1").arg(stars));

    const int usedSlots = static_cast<int>(unit->equippedItems().size());
    const int maxSlots = unit->equipSlotCount();
    equipSlotLabel_->setText(QString("装备槽: %1 / %2").arg(usedSlots).arg(maxSlots));

    const bool isPlayerUnit = (unit->owner() == core::UnitOwner::player);
    rebuildEquipSlotRows(unit, isPlayerUnit);

    // 攻击类型由中间层决定：法术攻击单位显示法攻，物理攻击单位显示物攻。
    if (unit->usesMagicAttack()) {
        atkLabel_->setText(QString("法攻: %1").arg(unit->magicAtk()));
        atkLabel_->setStyleSheet("font-size: 12px; color: #89DCEB;");
    } else {
        atkLabel_->setText(QString("物攻: %1").arg(unit->physicalAtk()));
        atkLabel_->setStyleSheet("font-size: 12px; color: #FAB387;");
    }
    // 物防与魔防始终显示（非零高亮，为零灰显），便于玩家了解完整防御属性。
    physDefLabel_->setText(QString("物防: %1").arg(unit->physicalDef()));
    physDefLabel_->setStyleSheet(unit->physicalDef() > 0
        ? "font-size: 12px; color: #A6E3A1;"
        : "font-size: 12px; color: #6C7086;");
    magDefLabel_->setText(QString("魔防: %1").arg(unit->magicDef()));
    magDefLabel_->setStyleSheet(unit->magicDef() > 0
        ? "font-size: 12px; color: #CBA6F7;"
        : "font-size: 12px; color: #6C7086;");
    atkSpeedLabel_->setText(QString("攻速: %1").arg(unit->attackSpeed()));
    atkSpeedLabel_->setStyleSheet(unit->attackSpeed() > 100
        ? "font-size: 12px; color: #F9E2AF;"
        : "font-size: 12px; color: #6C7086;");
    range_->setText(QString("射程: %1").arg(unit->attackRange()));
    hp_->setMaximum(unit->maxHp());
    hp_->setValue(unit->hp());
    mana_->setMaximum(unit->maxMana());
    mana_->setValue(unit->mana());

    sellBtn_->setEnabled(isPlayerUnit);
}

void UnitInfoPanel::onSellClicked() {
    if (currentUnitId_ >= 0) {
        emit sellRequested(currentUnitId_);
    }
}

void UnitInfoPanel::onUnequipSlotClicked() {
    if (currentUnitId_ < 0) {
        return;
    }
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    if (btn == nullptr) {
        return;
    }
    const int slotIndex = btn->property("slotIndex").toInt();
    emit unequipRequested(currentUnitId_, slotIndex);
}

// 流程：switch UnitClass ──> 返回中文职业名
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
