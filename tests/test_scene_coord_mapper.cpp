#include <gtest/gtest.h>

#include "ui/qt/SceneCoordMapper.h"

namespace my_auto_arena {
namespace ui {

TEST(SceneCoordMapperTest, MapsBoardCenterCorrectly) {
    SceneCoordMapper mapper(8, 8, 8, 64.0, 20.0, 20.0, 24.0);
    core::DragLocation location = core::DragLocation::fromBench(0);

    const bool ok = mapper.pixelToLocation(20.0 + 3.5 * 64.0, 20.0 + 4.5 * 64.0, location);

    EXPECT_TRUE(ok);
    EXPECT_EQ(location.type, core::DragLocation::kBoard);
    EXPECT_EQ(location.boardPos.row, 4);
    EXPECT_EQ(location.boardPos.col, 3);
}

TEST(SceneCoordMapperTest, MapsBenchCenterCorrectly) {
    SceneCoordMapper mapper(8, 8, 8, 64.0, 20.0, 20.0, 24.0);
    core::DragLocation location = core::DragLocation::fromBoard(0, 0);

    const bool ok = mapper.pixelToLocation(20.0 + 2.5 * 64.0, 20.0 + 8.0 * 64.0 + 24.0 + 32.0, location);

    EXPECT_TRUE(ok);
    EXPECT_EQ(location.type, core::DragLocation::kBench);
    EXPECT_EQ(location.benchIndex, 2);
}

TEST(SceneCoordMapperTest, ReturnsFalseForGap) {
    SceneCoordMapper mapper(8, 8, 8, 64.0, 20.0, 20.0, 24.0);
    core::DragLocation location = core::DragLocation::fromBoard(0, 0);

    const bool ok = mapper.pixelToLocation(20.0 + 1.0 * 64.0, 20.0 + 8.0 * 64.0 + 5.0, location);

    EXPECT_FALSE(ok);
}

TEST(SceneCoordMapperTest, RoundTripBoardLocation) {
    SceneCoordMapper mapper(8, 8, 8, 64.0, 20.0, 20.0, 24.0);
    const core::DragLocation original = core::DragLocation::fromBoard(7, 7);
    double x = 0.0;
    double y = 0.0;
    mapper.locationToPixelCenter(original, x, y);

    core::DragLocation mapped = core::DragLocation::fromBench(0);
    const bool ok = mapper.pixelToLocation(x, y, mapped);

    EXPECT_TRUE(ok);
    EXPECT_EQ(mapped.type, core::DragLocation::kBoard);
    EXPECT_EQ(mapped.boardPos.row, 7);
    EXPECT_EQ(mapped.boardPos.col, 7);
}

}  // namespace ui
}  // namespace my_auto_arena
