#include "core/GameFSM.h"

#include <stdexcept>

namespace my_auto_arena {
namespace core {

GameFSM::GameFSM()
    : phase_(GamePhase::kPrepare),
      round_(1),
      gameOver_(false),
      hasOutcome_(false),
      lastOutcome_{false, 0, 0, false, 0, 0, 0, 0} {}

GameFSM::GameFSM(const GameFSM& other)
    : phase_(other.phase_),
      round_(other.round_),
      gameOver_(other.gameOver_),
      hasOutcome_(other.hasOutcome_),
      lastOutcome_(other.lastOutcome_) {}

GamePhase GameFSM::currentPhase() const { return phase_; }

int GameFSM::currentRound() const { return round_; }

bool GameFSM::isGameOver() const { return gameOver_; }

bool GameFSM::canPlayerAct() const { return phase_ == GamePhase::kPrepare && !gameOver_; }

bool GameFSM::hasOutcome() const { return hasOutcome_; }

// 流程：校验未 gameOver ──> 校验当前为准备阶段 ──> 切换到战斗阶段
PhaseTransitionResult GameFSM::startBattle() {
    if (gameOver_) {
        return PhaseTransitionResult::kGameOver;
    }
    if (phase_ == GamePhase::kBattle) {
        return PhaseTransitionResult::kAlreadyInPhase;
    }
    if (phase_ != GamePhase::kPrepare) {
        return PhaseTransitionResult::kIllegalTransition;
    }
    phase_ = GamePhase::kBattle;
    return PhaseTransitionResult::kSuccess;
}

// 流程：校验未 gameOver ──> 校验当前为战斗阶段 ──> 保存 outcome ──> 切换到结算阶段
PhaseTransitionResult GameFSM::startSettlement(const RoundOutcome& outcome) {
    if (gameOver_) {
        return PhaseTransitionResult::kGameOver;
    }
    if (phase_ == GamePhase::kSettlement) {
        return PhaseTransitionResult::kAlreadyInPhase;
    }
    if (phase_ != GamePhase::kBattle) {
        return PhaseTransitionResult::kIllegalTransition;
    }
    lastOutcome_ = outcome;
    hasOutcome_ = true;
    phase_ = GamePhase::kSettlement;
    return PhaseTransitionResult::kSuccess;
}

// 流程：校验未 gameOver ──> 校验当前为结算阶段 ──> 切回准备阶段并 round+1
PhaseTransitionResult GameFSM::startNextRound() {
    if (gameOver_) {
        return PhaseTransitionResult::kGameOver;
    }
    if (phase_ == GamePhase::kPrepare) {
        return PhaseTransitionResult::kAlreadyInPhase;
    }
    if (phase_ != GamePhase::kSettlement) {
        return PhaseTransitionResult::kIllegalTransition;
    }
    phase_ = GamePhase::kPrepare;
    ++round_;
    return PhaseTransitionResult::kSuccess;
}

// 流程：检查是否已有结算结果 ──> 未结算则抛逻辑错误 ──> 已结算返回最近一轮 outcome 引用
const RoundOutcome& GameFSM::lastOutcome() const {
    if (!hasOutcome_) {
        throw std::logic_error("lastOutcome() called before any round has been settled; check hasOutcome() first.");
    }
    return lastOutcome_;
}

void GameFSM::setGameOver() { gameOver_ = true; }

}  // namespace core
}  // namespace my_auto_arena
