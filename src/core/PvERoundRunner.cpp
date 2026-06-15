#include "core/PvERoundRunner.h"

#include "core/BattleEngine.h"
#include "core/EconomySystem.h"
#include "core/Unit.h"

namespace my_auto_arena {
namespace core {

// 流程：读取关卡配置 ──> 生成敌方并注册 ──> 驱动 BattleEngine 至结束
//       ──> 结算金币/扣血 ──> 释放阵亡敌方内存 ──> 清理存活敌方
RoundOutcome PvERoundRunner::runRoundBattle(Board& board, Player& player, std::map<int, Unit*>& units, int roundIndex,
                                            EnemySpawner& spawner, int& nextUnitId) {
    const LevelConfig cfg = spawner.configForRound(roundIndex);
    // PvERoundRunner 负责 spawned 列表中所有指针的生命周期（new 由 EnemySpawner::spawnRound 执行）。
    std::vector<Unit*> spawned = spawner.spawnRound(roundIndex, board, nextUnitId);
    for (std::size_t i = 0; i < spawned.size(); ++i) {
        Unit* unit = spawned.at(i);
        units[unit->id()] = unit;
    }

    BattleEngine engine(board, units);
    engine.setDefeatHpPenalty(cfg.onLosePlayerHpDamage);
    while (!engine.isFinished()) {
        engine.tick();
    }

    RoundOutcome outcome = engine.outcome();
    if (outcome.playerWon) {
        const EconomyBreakdown economy = EconomySystem::settleRound(player, true, cfg.winGoldReward);
        outcome.goldReward = economy.totalGold;
        outcome.baseGoldReward = economy.baseGold;
        outcome.interestGold = economy.interestGold;
        outcome.winStreakBonus = economy.winStreakBonus;
        outcome.winStreak = economy.winStreak;
        outcome.gameOver = false;
    } else {
        const EconomyBreakdown economy = EconomySystem::settleRound(player, false, cfg.winGoldReward);
        outcome.goldReward = economy.totalGold;
        outcome.baseGoldReward = economy.baseGold;
        outcome.interestGold = economy.interestGold;
        outcome.winStreakBonus = economy.winStreakBonus;
        outcome.winStreak = economy.winStreak;
        outcome.hpPenalty = cfg.onLosePlayerHpDamage;
        const int nextHp = player.hp() - outcome.hpPenalty;
        player.setHp(nextHp < 0 ? 0 : nextHp);
        // 血量归零时标记游戏结束，调用方应据此调用 GameFSM::setGameOver()。
        outcome.gameOver = (player.hp() <= 0);
    }

    // 释放所有已生成单位的指针：
    //   - 战斗中已死亡的敌方单位已被 BattleEngine::clearDeadUnits 从 units 中移除（但未 delete）
    //   - 战斗后仍存活的敌方单位由 removeEnemyUnits 清理 Board 并 delete
    // 因此死亡单位需在这里补充 delete。
    for (std::size_t i = 0; i < spawned.size(); ++i) {
        Unit* unit = spawned.at(i);
        if (units.find(unit->id()) == units.end()) {
            // 已被 clearDeadUnits 从 units 移除（战斗中阵亡），Board 已清，直接释放。
            delete unit;
        }
    }
    removeEnemyUnits(board, units);
    return outcome;
}

// 流程：遍历 units_ ──> 筛出敌方单位 ──> 清空棋盘占位 ──> delete 并 erase
void PvERoundRunner::removeEnemyUnits(Board& board, std::map<int, Unit*>& units) {
    for (std::map<int, Unit*>::iterator it = units.begin(); it != units.end();) {
        Unit* unit = it->second;
        if (unit == nullptr || unit->owner() != UnitOwner::enemy) {
            ++it;
            continue;
        }
        const Position pos = board.findUnitOnBoard(unit->id());
        if (board.inBounds(pos)) {
            board.clearOnBoard(pos);
        }
        delete unit;
        it = units.erase(it);
    }
}

}  // namespace core
}  // namespace my_auto_arena
