#ifndef MY_AUTO_ARENA_UI_QT_UNIT_INFO_PANEL_H
#define MY_AUTO_ARENA_UI_QT_UNIT_INFO_PANEL_H

#include <QLabel>
#include <QProgressBar>
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

private:
    QLabel* name_;
    QLabel* attack_;
    QLabel* range_;
    QProgressBar* hp_;
    QProgressBar* mana_;
};

}  // namespace ui
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_UI_QT_UNIT_INFO_PANEL_H
