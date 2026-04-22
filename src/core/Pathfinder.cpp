#include "core/Pathfinder.h"

#include <algorithm>
#include <queue>
#include <vector>

namespace my_auto_arena {
namespace core {

namespace {

bool inAttackRange(Position p, Position targetPos, int attackRange) {
    const int dr = p.row - targetPos.row;
    const int dc = p.col - targetPos.col;
    return dr * dr + dc * dc <= attackRange * attackRange;
}

// 判断单位是否可进入目标格：仅允许空格。
// 注：起始格在 BFS 开始前已标记 visited，永远不会被扩展为邻格，故无需特判自身起始位置。
bool canEnterCell(const Board& board, Position cell) {
    return board.occupantOnBoard(cell) == Board::kEmptySlot;
}

bool isGoalCell(const Board& board, Position cell, Position targetPos, int attackRange) {
    if (!board.inBounds(cell)) {
        return false;
    }
    if (board.occupantOnBoard(cell) != Board::kEmptySlot) {
        return false;
    }
    return inAttackRange(cell, targetPos, attackRange);
}

}  // namespace

bool Pathfinder::nextStepTowardAttackRange(const Board& board,
                                           const std::map<int, Unit*>& units,  // 预留参数：用于未来按单位类型设置通行性
                                           int movingUnitId, Position start, Position targetPos, int attackRange,
                                           Position* outNext) {
    (void)units;
    (void)movingUnitId;
    if (outNext == nullptr || !board.inBounds(start) || !board.inBounds(targetPos)) {
        return false;
    }
    if (attackRange <= 0) {
        return false;
    }

    const int rows = board.rows();
    const int cols = board.cols();
    std::vector<std::vector<bool> > visited(rows, std::vector<bool>(cols, false));
    std::vector<std::vector<int> > parentRow(rows, std::vector<int>(cols, -1));
    std::vector<std::vector<int> > parentCol(rows, std::vector<int>(cols, -1));

    std::queue<Position> q;
    visited.at(start.row).at(start.col) = true;
    q.push(start);

    Position goal{-1, -1};
    while (!q.empty()) {
        const Position cur = q.front();
        q.pop();
        if (isGoalCell(board, cur, targetPos, attackRange)) {
            goal = cur;
            break;
        }
        const int dirs[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
        for (int d = 0; d < 4; ++d) {
            const Position nxt{cur.row + dirs[d][0], cur.col + dirs[d][1]};
            if (!board.inBounds(nxt)) {
                continue;
            }
            if (!canEnterCell(board, nxt)) {
                continue;
            }
            if (visited.at(nxt.row).at(nxt.col)) {
                continue;
            }
            visited.at(nxt.row).at(nxt.col) = true;
            parentRow.at(nxt.row).at(nxt.col) = cur.row;
            parentCol.at(nxt.row).at(nxt.col) = cur.col;
            q.push(nxt);
        }
    }

    if (goal.row < 0) {
        return false;
    }

    std::vector<Position> backwards;
    Position cur = goal;
    while (true) {
        backwards.push_back(cur);
        if (cur.row == start.row && cur.col == start.col) {
            break;
        }
        const int pr = parentRow.at(cur.row).at(cur.col);
        const int pc = parentCol.at(cur.row).at(cur.col);
        if (pr < 0 || pc < 0) {
            return false;
        }
        cur.row = pr;
        cur.col = pc;
    }
    std::reverse(backwards.begin(), backwards.end());
    if (backwards.size() < 2) {
        return false;
    }
    *outNext = backwards.at(1);
    return true;
}

}  // namespace core
}  // namespace my_auto_arena
