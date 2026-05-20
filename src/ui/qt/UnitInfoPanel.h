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
    // 玩家点击出售按钮时发出，携带当前单位 ID。
    void sellRequested(int unitId);

private slots:
    void onSellClicked();

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

    int currentUnitId_;  // 当前展示的单位 ID，-1 表示无单位

    // 将职业枚举转换为中文名称。
    static QString unitClassName(core::UnitClass cls);
};

}  // namespace ui
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_UI_QT_UNIT_INFO_PANEL_H
