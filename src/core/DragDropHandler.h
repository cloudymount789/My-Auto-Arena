#ifndef MY_AUTO_ARENA_CORE_DRAG_DROP_HANDLER_H
#define MY_AUTO_ARENA_CORE_DRAG_DROP_HANDLER_H

#include "core/Board.h"
#include "core/Player.h"

namespace my_auto_arena {
namespace core {

struct DragLocation {
    enum Type { kBoard, kBench };

    Type type;
    Position boardPos;
    int benchIndex;

    static DragLocation fromBoard(int row, int col);
    static DragLocation fromBench(int index);
};

enum class DragResult {
    kSuccess,
    kSwapped,
    kInvalidSource,
    kOutOfBounds,
    kSameLocation,
    kNotPlayerHalf,
    kPopulationFull
};

class DragDropHandler {
public:
    explicit DragDropHandler(Board& board);
    DragDropHandler(Board& board, const Player& player);

    DragResult execute(const DragLocation& from, const DragLocation& to);

private:
    Board& board_;
    const Player* player_;

    int pickUnit(const DragLocation& location) const;
    bool placeUnit(int unitId, const DragLocation& location);
    void clearUnit(const DragLocation& location);
    bool isValidLocation(const DragLocation& location) const;
    bool isSameLocation(const DragLocation& from, const DragLocation& to) const;
    DragResult checkPlayerConstraints(const DragLocation& from, const DragLocation& to, int targetUnitId) const;
};

}  // namespace core
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_CORE_DRAG_DROP_HANDLER_H
