#include "core/DragDropHandler.h"

namespace my_auto_arena {
namespace core {

DragLocation DragLocation::fromBoard(int row, int col) {
    DragLocation location;
    location.type = kBoard;
    location.boardPos.row = row;
    location.boardPos.col = col;
    location.benchIndex = -1;
    return location;
}

DragLocation DragLocation::fromBench(int index) {
    DragLocation location;
    location.type = kBench;
    location.boardPos.row = -1;
    location.boardPos.col = -1;
    location.benchIndex = index;
    return location;
}

DragDropHandler::DragDropHandler(Board& board) : board_(board), player_(nullptr) {}

DragDropHandler::DragDropHandler(Board& board, const Player& player) : board_(board), player_(&player) {}

DragResult DragDropHandler::execute(const DragLocation& from, const DragLocation& to) {
    if (isSameLocation(from, to)) {
        return DragResult::kSameLocation;
    }
    if (!isValidLocation(to)) {
        return DragResult::kOutOfBounds;
    }

    const int sourceUnitId = pickUnit(from);
    if (sourceUnitId < 0) {
        return DragResult::kInvalidSource;
    }

    const int targetUnitId = pickUnit(to);
    const DragResult constraintResult = checkPlayerConstraints(from, to, targetUnitId);
    if (constraintResult != DragResult::kSuccess) {
        return constraintResult;
    }

    clearUnit(from);

    if (targetUnitId < 0) {
        if (!placeUnit(sourceUnitId, to)) {
            placeUnit(sourceUnitId, from);
            return DragResult::kOutOfBounds;
        }
        return DragResult::kSuccess;
    }

    clearUnit(to);
    const bool placeSourceOk = placeUnit(sourceUnitId, to);
    const bool placeTargetOk = placeUnit(targetUnitId, from);
    if (!placeSourceOk || !placeTargetOk) {
        clearUnit(to);
        clearUnit(from);
        // 回滚交换：若恢复失败，单位状态仍然一致（格子已清空），不会产生悬空引用。
        const bool restoredSrc = placeUnit(sourceUnitId, from);
        const bool restoredTgt = placeUnit(targetUnitId, to);
        (void)restoredSrc;
        (void)restoredTgt;
        return DragResult::kOutOfBounds;
    }
    return DragResult::kSwapped;
}

int DragDropHandler::pickUnit(const DragLocation& location) const {
    if (location.type == DragLocation::kBoard) {
        return board_.occupantOnBoard(location.boardPos);
    }
    return board_.occupantOnBench(location.benchIndex);
}

bool DragDropHandler::placeUnit(int unitId, const DragLocation& location) {
    if (location.type == DragLocation::kBoard) {
        return board_.placeOnBoard(unitId, location.boardPos);
    }
    return board_.placeOnBench(unitId, location.benchIndex);
}

void DragDropHandler::clearUnit(const DragLocation& location) {
    if (location.type == DragLocation::kBoard) {
        board_.clearOnBoard(location.boardPos);
    } else {
        board_.clearOnBench(location.benchIndex);
    }
}

bool DragDropHandler::isValidLocation(const DragLocation& location) const {
    if (location.type == DragLocation::kBoard) {
        return board_.inBounds(location.boardPos);
    }
    return location.benchIndex >= 0 && location.benchIndex < board_.benchSize();
}

bool DragDropHandler::isSameLocation(const DragLocation& from, const DragLocation& to) const {
    if (from.type != to.type) {
        return false;
    }
    if (from.type == DragLocation::kBoard) {
        return from.boardPos.row == to.boardPos.row && from.boardPos.col == to.boardPos.col;
    }
    return from.benchIndex == to.benchIndex;
}

DragResult DragDropHandler::checkPlayerConstraints(const DragLocation& from, const DragLocation& to,
                                                   int targetUnitId) const {
    if (player_ == nullptr) {
        return DragResult::kSuccess;
    }

    const int sourceUnitId = pickUnit(from);
    if (!player_->ownsUnit(sourceUnitId)) {
        return DragResult::kInvalidSource;
    }

    if (to.type == DragLocation::kBoard && !board_.isPlayerHalf(to.boardPos)) {
        return DragResult::kNotPlayerHalf;
    }

    if (from.type == DragLocation::kBench && to.type == DragLocation::kBoard) {
        const int currentPopulation = player_->populationOnBoard(board_);
        const int netIncrease = (targetUnitId >= 0) ? 0 : 1;
        if (currentPopulation + netIncrease > player_->populationCap()) {
            return DragResult::kPopulationFull;
        }
    }

    return DragResult::kSuccess;
}

}  // namespace core
}  // namespace my_auto_arena
