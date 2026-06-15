#include "core/DragDropHandler.h"

namespace my_auto_arena {
namespace core {

// 流程：构造棋盘位置对象 ──> 写入 row/col ──> benchIndex 置为无效值
DragLocation DragLocation::fromBoard(int row, int col) {
    DragLocation location;
    location.type = kBoard;
    location.boardPos.row = row;
    location.boardPos.col = col;
    location.benchIndex = -1;
    return location;
}

// 流程：构造备战区位置对象 ──> 写入 benchIndex ──> row/col 置为无效值
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

// 流程：校验起止位置 ──> 取源/目标单位 ──> 检查玩家约束 ──> 清空源格
//       ──> 目标空：放置源单位 ──> 目标有单位：交换两格（失败则回滚）
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

// 流程：按位置类型读取棋盘或备战区占用 ──> 返回单位 ID 或空槽标记
int DragDropHandler::pickUnit(const DragLocation& location) const {
    if (location.type == DragLocation::kBoard) {
        return board_.occupantOnBoard(location.boardPos);
    }
    return board_.occupantOnBench(location.benchIndex);
}

// 流程：按位置类型放入棋盘或备战区 ──> 返回底层 Board 放置结果
bool DragDropHandler::placeUnit(int unitId, const DragLocation& location) {
    if (location.type == DragLocation::kBoard) {
        return board_.placeOnBoard(unitId, location.boardPos);
    }
    return board_.placeOnBench(unitId, location.benchIndex);
}

// 流程：按位置类型清理棋盘或备战区占位 ──> 备战区以外类型按备战区下标处理
void DragDropHandler::clearUnit(const DragLocation& location) {
    if (location.type == DragLocation::kBoard) {
        board_.clearOnBoard(location.boardPos);
    } else {
        board_.clearOnBench(location.benchIndex);
    }
}

// 流程：按位置类型校验棋盘坐标或备战区下标 ──> 返回位置是否可用于拖放
bool DragDropHandler::isValidLocation(const DragLocation& location) const {
    if (location.type == DragLocation::kBoard) {
        return board_.inBounds(location.boardPos);
    }
    return location.benchIndex >= 0 && location.benchIndex < board_.benchSize();
}

// 流程：先比较位置类型 ──> 棋盘位置比较 row/col ──> 备战区位置比较 slot
bool DragDropHandler::isSameLocation(const DragLocation& from, const DragLocation& to) const {
    if (from.type != to.type) {
        return false;
    }
    if (from.type == DragLocation::kBoard) {
        return from.boardPos.row == to.boardPos.row && from.boardPos.col == to.boardPos.col;
    }
    return from.benchIndex == to.benchIndex;
}

// 流程：无玩家则放行 ──> 校验源单位归属 ──> 禁止拖到敌方半场 ──> 备战区上场检查人口上限
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
