#include <gtest/gtest.h>

#include "core/GameFSM.h"

namespace my_auto_arena {
namespace core {

TEST(GameFSMTest, InitialStateIsPrepareRoundOne) {
    GameFSM fsm;
    EXPECT_EQ(fsm.currentPhase(), GamePhase::kPrepare);
    EXPECT_EQ(fsm.currentRound(), 1);
    EXPECT_FALSE(fsm.isGameOver());
    EXPECT_TRUE(fsm.canPlayerAct());
    EXPECT_FALSE(fsm.hasOutcome());
}

TEST(GameFSMTest, LegalPhaseLoopWorks) {
    GameFSM fsm;
    EXPECT_EQ(fsm.startBattle(), PhaseTransitionResult::kSuccess);
    EXPECT_EQ(fsm.currentPhase(), GamePhase::kBattle);
    EXPECT_FALSE(fsm.canPlayerAct());

    RoundOutcome outcome{true, 5, 0};
    EXPECT_EQ(fsm.startSettlement(outcome), PhaseTransitionResult::kSuccess);
    EXPECT_EQ(fsm.currentPhase(), GamePhase::kSettlement);
    EXPECT_FALSE(fsm.canPlayerAct());
    EXPECT_TRUE(fsm.lastOutcome().playerWon);
    EXPECT_EQ(fsm.lastOutcome().goldReward, 5);
    EXPECT_EQ(fsm.lastOutcome().hpPenalty, 0);
    EXPECT_TRUE(fsm.hasOutcome());

    EXPECT_EQ(fsm.startNextRound(), PhaseTransitionResult::kSuccess);
    EXPECT_EQ(fsm.currentPhase(), GamePhase::kPrepare);
    EXPECT_EQ(fsm.currentRound(), 2);
    EXPECT_TRUE(fsm.canPlayerAct());
}

TEST(GameFSMTest, RejectsIllegalTransitions) {
    GameFSM fsm;
    RoundOutcome lose{false, 1, 4};

    EXPECT_EQ(fsm.startSettlement(lose), PhaseTransitionResult::kIllegalTransition);
    EXPECT_EQ(fsm.currentPhase(), GamePhase::kPrepare);
    EXPECT_EQ(fsm.currentRound(), 1);
    EXPECT_EQ(fsm.startNextRound(), PhaseTransitionResult::kAlreadyInPhase);
    EXPECT_EQ(fsm.currentPhase(), GamePhase::kPrepare);
    EXPECT_EQ(fsm.startBattle(), PhaseTransitionResult::kSuccess);
    EXPECT_EQ(fsm.currentPhase(), GamePhase::kBattle);
    EXPECT_EQ(fsm.startBattle(), PhaseTransitionResult::kAlreadyInPhase);
    EXPECT_EQ(fsm.currentPhase(), GamePhase::kBattle);
    EXPECT_EQ(fsm.startNextRound(), PhaseTransitionResult::kIllegalTransition);
    EXPECT_EQ(fsm.currentPhase(), GamePhase::kBattle);
    EXPECT_EQ(fsm.startSettlement(lose), PhaseTransitionResult::kSuccess);
    EXPECT_EQ(fsm.currentPhase(), GamePhase::kSettlement);
    EXPECT_EQ(fsm.startSettlement(lose), PhaseTransitionResult::kAlreadyInPhase);
    EXPECT_EQ(fsm.currentPhase(), GamePhase::kSettlement);
}

TEST(GameFSMTest, GameOverBlocksAnyTransition) {
    GameFSM fsm;
    fsm.setGameOver();
    RoundOutcome outcome{false, 0, 10};

    EXPECT_EQ(fsm.startBattle(), PhaseTransitionResult::kGameOver);
    EXPECT_EQ(fsm.startSettlement(outcome), PhaseTransitionResult::kGameOver);
    EXPECT_EQ(fsm.startNextRound(), PhaseTransitionResult::kGameOver);
    EXPECT_TRUE(fsm.isGameOver());
}

TEST(GameFSMTest, CopyConstructorPreservesState) {
    GameFSM fsm;
    EXPECT_EQ(fsm.startBattle(), PhaseTransitionResult::kSuccess);
    RoundOutcome outcome{false, 2, 3};
    EXPECT_EQ(fsm.startSettlement(outcome), PhaseTransitionResult::kSuccess);

    GameFSM copy(fsm);
    EXPECT_EQ(copy.currentPhase(), GamePhase::kSettlement);
    EXPECT_EQ(copy.currentRound(), 1);
    EXPECT_EQ(copy.lastOutcome().goldReward, 2);
    EXPECT_EQ(copy.lastOutcome().hpPenalty, 3);
    EXPECT_FALSE(copy.canPlayerAct());
    EXPECT_TRUE(copy.hasOutcome());
}

TEST(GameFSMTest, CopyConstructorPreservesGameOverState) {
    GameFSM fsm;
    fsm.setGameOver();
    GameFSM copy(fsm);
    EXPECT_TRUE(copy.isGameOver());
    EXPECT_EQ(copy.startBattle(), PhaseTransitionResult::kGameOver);
}

TEST(GameFSMTest, MultiRoundLoopIncrementsRoundMonotonically) {
    GameFSM fsm;
    for (int i = 0; i < 3; ++i) {
        EXPECT_EQ(fsm.startBattle(), PhaseTransitionResult::kSuccess);
        EXPECT_EQ(fsm.startSettlement(RoundOutcome{true, 3, 0}), PhaseTransitionResult::kSuccess);
        EXPECT_EQ(fsm.startNextRound(), PhaseTransitionResult::kSuccess);
        EXPECT_EQ(fsm.currentRound(), i + 2);
        EXPECT_EQ(fsm.currentPhase(), GamePhase::kPrepare);
    }
}

}  // namespace core
}  // namespace my_auto_arena
