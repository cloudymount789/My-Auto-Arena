#include "ui/qt/QtMainWindow.h"

#include <QGraphicsView>
#include <QHBoxLayout>
#include <QPainter>
#include <QStatusBar>
#include <QWidget>

#include "ui/qt/ArenaScene.h"
#include "ui/qt/UnitInfoPanel.h"

namespace my_auto_arena {
namespace ui {

QtMainWindow::QtMainWindow(QWidget* parent)
    : QMainWindow(parent), board_(8, 8, 8), player_(1, 10, 100, 1, 3), scene_(nullptr), view_(nullptr), infoPanel_(nullptr) {
    player_.addUnit(1);
    player_.addUnit(2);
    player_.addUnit(3);

    units_.push_back(new core::WarriorUnit(1, core::UnitOwner::player));
    units_.push_back(new core::MageUnit(2, core::UnitOwner::player));
    units_.push_back(new core::WarriorUnit(3, core::UnitOwner::player));

    board_.placeOnBench(1, 0);
    board_.placeOnBench(2, 1);
    board_.placeOnBench(3, 2);

    QWidget* central = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(central);
    setCentralWidget(central);

    scene_ = new ArenaScene(board_, player_, units_, this);
    view_ = new QGraphicsView(scene_, central);
    view_->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    infoPanel_ = new UnitInfoPanel(central);
    infoPanel_->setFixedWidth(220);

    layout->addWidget(view_, 1);
    layout->addWidget(infoPanel_);

    connect(scene_, SIGNAL(unitSelected(int)), this, SLOT(onUnitSelected(int)));
    connect(scene_, SIGNAL(dragResultReady(core::DragResult)), this, SLOT(onDragResult(core::DragResult)));

    setWindowTitle("Synera Qt GUI (Phase 1)");
    resize(940, 760);
    statusBar()->showMessage("Ready");
}

QtMainWindow::~QtMainWindow() {
    for (std::size_t i = 0; i < units_.size(); ++i) {
        delete units_[i];
    }
}

void QtMainWindow::onUnitSelected(int unitId) { infoPanel_->setUnit(scene_->unitById(unitId)); }

void QtMainWindow::onDragResult(core::DragResult result) {
    if (result == core::DragResult::kSuccess) {
        statusBar()->showMessage("放置成功", 1500);
    } else if (result == core::DragResult::kSwapped) {
        statusBar()->showMessage("交换成功", 1500);
    } else if (result == core::DragResult::kNotPlayerHalf) {
        statusBar()->showMessage("非法位置：非玩家半场", 1800);
    } else if (result == core::DragResult::kPopulationFull) {
        statusBar()->showMessage("人口已满", 1800);
    } else if (result == core::DragResult::kOutOfBounds) {
        statusBar()->showMessage("越界：已回弹", 1800);
    } else if (result == core::DragResult::kSameLocation) {
        statusBar()->showMessage("原地放下", 1200);
    } else {
        statusBar()->showMessage("非法操作", 1500);
    }
}

}  // namespace ui
}  // namespace my_auto_arena
