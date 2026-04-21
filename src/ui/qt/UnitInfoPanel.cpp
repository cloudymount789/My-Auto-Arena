#include "ui/qt/UnitInfoPanel.h"

#include <QVBoxLayout>

namespace my_auto_arena {
namespace ui {

UnitInfoPanel::UnitInfoPanel(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    name_ = new QLabel("Name: -", this);
    attack_ = new QLabel("ATK: -", this);
    range_ = new QLabel("Range: -", this);
    hp_ = new QProgressBar(this);
    mana_ = new QProgressBar(this);
    hp_->setFormat("HP %v/%m");
    mana_->setFormat("Mana %v/%m");
    layout->addWidget(name_);
    layout->addWidget(attack_);
    layout->addWidget(range_);
    layout->addWidget(hp_);
    layout->addWidget(mana_);
    layout->addStretch();
}

void UnitInfoPanel::setUnit(const core::Unit* unit) {
    if (unit == nullptr) {
        name_->setText("Name: -");
        attack_->setText("ATK: -");
        range_->setText("Range: -");
        hp_->setMaximum(1);
        hp_->setValue(0);
        mana_->setMaximum(1);
        mana_->setValue(0);
        return;
    }
    name_->setText(QString("Name: %1").arg(QString::fromStdString(unit->name())));
    attack_->setText(QString("ATK: %1").arg(unit->attack()));
    range_->setText(QString("Range: %1").arg(unit->attackRange()));
    hp_->setMaximum(unit->maxHp());
    hp_->setValue(unit->hp());
    mana_->setMaximum(unit->maxMana());
    mana_->setValue(unit->mana());
}

}  // namespace ui
}  // namespace my_auto_arena
