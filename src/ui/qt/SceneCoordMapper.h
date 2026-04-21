#ifndef MY_AUTO_ARENA_UI_QT_SCENE_COORD_MAPPER_H
#define MY_AUTO_ARENA_UI_QT_SCENE_COORD_MAPPER_H

#include "core/DragDropHandler.h"

namespace my_auto_arena {
namespace ui {

class SceneCoordMapper {
public:
    SceneCoordMapper(int boardRows, int boardCols, int benchSize, double tileSize,
                     double boardOffsetX, double boardOffsetY, double benchGap);

    bool pixelToLocation(double sceneX, double sceneY, core::DragLocation& outLocation) const;
    void locationToPixelCenter(const core::DragLocation& location, double& outX, double& outY) const;

private:
    int boardRows_;
    int boardCols_;
    int benchSize_;
    double tileSize_;
    double boardOffsetX_;
    double boardOffsetY_;
    double benchGap_;
};

}  // namespace ui
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_UI_QT_SCENE_COORD_MAPPER_H
