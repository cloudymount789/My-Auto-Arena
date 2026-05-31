#ifndef MY_AUTO_ARENA_UI_QT_UNIT_INFO_PANEL_H
#define MY_AUTO_ARENA_UI_QT_UNIT_INFO_PANEL_H

#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include "core/Unit.h"

namespace my_auto_arena {
namespace ui {

class UnitInfoPanel : public QWidget {
    Q_OBJECT
public:
    explicit UnitInfoPanel(QWidget* parent);

public slots:
    void setUnit(const core::Unit* unit);

signals:
    void sellRequested(int unitId);              // 点击「出售」
    void unequipRequested(int unitId, int slotIndex);  // 卸下指定槽位装备

private slots:
    void onSellClicked();
    void onUnequipSlotClicked();

private:
    QLabel* name_;
    QLabel* classLabel_;
    QLabel* starLabel_;
    QLabel* equipSlotLabel_;
    QWidget* equipSlotsWidget_;
    QVBoxLayout* equipSlotsLayout_;
    QLabel* atkLabel_;      // 显示物攻或法攻（根据职业类型）
    QLabel* physDefLabel_;  // 物理防御
    QLabel* magDefLabel_;   // 法术防御
    QLabel* atkSpeedLabel_; // 攻击速度（现有属性）
    QLabel* range_;
    QLabel* skillLabel_;
    QProgressBar* hp_;
    QProgressBar* mana_;
    QPushButton* sellBtn_;

    int currentUnitId_;  // 当前展示的单位 ID，-1 表示无单位

    void clearEquipSlotRows();
    void rebuildEquipSlotRows(const core::Unit* unit, bool isPlayerUnit);

    // 将职业枚举转换为中文名称。
    static QString unitClassName(core::UnitClass cls);
};

}  // namespace ui
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_UI_QT_UNIT_INFO_PANEL_H
