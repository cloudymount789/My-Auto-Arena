#include "core/StarUpgrade.h"

namespace my_auto_arena {
namespace core {

bool StarUpgrade::tryMergeAll(std::vector<Unit*>& playerUnits, Board& board,
                               std::map<int, Unit*>& unitsMap, Player& player) {
    bool anyMerge = false;
    bool merged = true;

    // 循环直到本轮没有合并发生（升2星可能触发升3星）。
    while (merged) {
        merged = false;

        for (std::size_t i = 0; i < playerUnits.size(); ++i) {
            Unit* base = playerUnits.at(i);
            if (base == nullptr || base->starLevel() >= 3) {
                continue;
            }

            // 找到与 base 同名同星级的其余单位。
            std::vector<int> sameIds;
            for (std::size_t j = 0; j < playerUnits.size(); ++j) {
                if (j == i) continue;
                Unit* other = playerUnits.at(j);
                if (other == nullptr) continue;
                if (other->name() == base->name() && other->starLevel() == base->starLevel()) {
                    sameIds.push_back(other->id());
                    if (sameIds.size() == 2) break;
                }
            }

            if (sameIds.size() < 2) {
                continue;
            }

            // 找到 3 张同卡，将 base 升星并移除另外 2 张。
            base->upgradeToStar(base->starLevel() + 1);
            removeUnit(sameIds.at(0), playerUnits, board, unitsMap, player);
            removeUnit(sameIds.at(1), playerUnits, board, unitsMap, player);

            merged = true;
            anyMerge = true;
            // 移除操作改变了 playerUnits，重新从头扫描。
            break;
        }
    }

    return anyMerge;
}

void StarUpgrade::removeUnit(int unitId, std::vector<Unit*>& playerUnits,
                              Board& board, std::map<int, Unit*>& unitsMap, Player& player) {
    // 从棋盘上移除占位。
    const Position pos = board.findUnitOnBoard(unitId);
    if (board.inBounds(pos)) {
        board.clearOnBoard(pos);
    }
    // 从备战区移除占位。
    for (int slot = 0; slot < board.benchSize(); ++slot) {
        if (board.occupantOnBench(slot) == unitId) {
            board.clearOnBench(slot);
            break;
        }
    }
    // 从玩家单位列表中移除并释放内存。
    for (std::size_t i = 0; i < playerUnits.size(); ++i) {
        if (playerUnits.at(i) != nullptr && playerUnits.at(i)->id() == unitId) {
            delete playerUnits.at(i);
            playerUnits.at(i) = nullptr;
            playerUnits.erase(playerUnits.begin() + static_cast<int>(i));
            break;
        }
    }
    // 从全局单位表中移除（不重复 delete，已在上方释放）。
    unitsMap.erase(unitId);
    // 从玩家拥有记录中移除。
    player.removeUnit(unitId);
}

}  // namespace core
}  // namespace my_auto_arena
