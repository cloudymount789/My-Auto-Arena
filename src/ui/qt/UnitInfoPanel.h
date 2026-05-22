#ifndef MY_AUTO_ARENA_UI_QT_UNIT_INFO_PANEL_H
#define MY_AUTO_ARENA_UI_QT_UNIT_INFO_PANEL_H

#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
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
    void sellRequested(int unitId);    // 点击「出售」
    void unequipRequested(int unitId); // 点击「卸装备」

private slots:
    void onSellClicked();
    void onUnequipClicked();

private:
    QLabel* name_;
    QLabel* classLabel_;
    QLabel* starLabel_;
    QLabel* itemLabel_;
    QLabel* attack_;
    QLabel* range_;
    QProgressBar* hp_;
    QProgressBar* mana_;
    QPushButton* sellBtn_;
    QPushButton* unequipBtn_;

    int currentUnitId_;  // 当前展示的单位 ID，-1 表示无单位

    // 将职业枚举转换为中文名称。
    static QString unitClassName(core::UnitClass cls);
};

}  // namespace ui
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_UI_QT_UNIT_INFO_PANEL_H
