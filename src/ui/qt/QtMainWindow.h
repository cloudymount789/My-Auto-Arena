#ifndef MY_AUTO_ARENA_UI_QT_QT_MAIN_WINDOW_H
#define MY_AUTO_ARENA_UI_QT_QT_MAIN_WINDOW_H

#include <vector>

#include <QGraphicsView>
#include <QMainWindow>

#include "core/Board.h"
#include "core/DragDropHandler.h"
#include "core/Player.h"
#include "core/Unit.h"

namespace my_auto_arena {
namespace ui {

class ArenaScene;
class UnitInfoPanel;

class QtMainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit QtMainWindow(QWidget* parent);
    virtual ~QtMainWindow() override;

private slots:
    void onUnitSelected(int unitId);
    void onDragResult(core::DragResult result);

private:
    core::Board board_;
    core::Player player_;
    std::vector<core::Unit*> units_;

    ArenaScene* scene_;
    QGraphicsView* view_;
    UnitInfoPanel* infoPanel_;
};

}  // namespace ui
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_UI_QT_QT_MAIN_WINDOW_H
