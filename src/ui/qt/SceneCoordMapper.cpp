#include "ui/qt/SceneCoordMapper.h"

namespace my_auto_arena {
namespace ui {

SceneCoordMapper::SceneCoordMapper(int boardRows, int boardCols, int benchSize, double tileSize,
                                   double boardOffsetX, double boardOffsetY, double benchGap)
    : boardRows_(boardRows),
      boardCols_(boardCols),
      benchSize_(benchSize),
      tileSize_(tileSize),
      boardOffsetX_(boardOffsetX),
      boardOffsetY_(boardOffsetY),
      benchGap_(benchGap) {}

bool SceneCoordMapper::pixelToLocation(double sceneX, double sceneY, core::DragLocation& outLocation) const {
    const double boardWidth = boardCols_ * tileSize_;
    const double boardHeight = boardRows_ * tileSize_;
    const double benchOriginY = boardOffsetY_ + boardHeight + benchGap_;

    if (sceneX >= boardOffsetX_ && sceneX < boardOffsetX_ + boardWidth && sceneY >= boardOffsetY_ &&
        sceneY < boardOffsetY_ + boardHeight) {
        const int col = static_cast<int>((sceneX - boardOffsetX_) / tileSize_);
        const int row = static_cast<int>((sceneY - boardOffsetY_) / tileSize_);
        outLocation = core::DragLocation::fromBoard(row, col);
        return true;
    }

    if (sceneX >= boardOffsetX_ && sceneX < boardOffsetX_ + boardWidth && sceneY >= benchOriginY &&
        sceneY < benchOriginY + tileSize_) {
        const int index = static_cast<int>((sceneX - boardOffsetX_) / tileSize_);
        if (index >= 0 && index < benchSize_) {
            outLocation = core::DragLocation::fromBench(index);
            return true;
        }
    }

    return false;
}

void SceneCoordMapper::locationToPixelCenter(const core::DragLocation& location, double& outX, double& outY) const {
    const double benchOriginY = boardOffsetY_ + boardRows_ * tileSize_ + benchGap_;

    if (location.type == core::DragLocation::kBoard) {
        outX = boardOffsetX_ + (location.boardPos.col + 0.5) * tileSize_;
        outY = boardOffsetY_ + (location.boardPos.row + 0.5) * tileSize_;
        return;
    }

    outX = boardOffsetX_ + (location.benchIndex + 0.5) * tileSize_;
    outY = benchOriginY + 0.5 * tileSize_;
}

}  // namespace ui
}  // namespace my_auto_arena
