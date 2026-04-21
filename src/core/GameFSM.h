#ifndef MY_AUTO_ARENA_CORE_GAME_FSM_H
#define MY_AUTO_ARENA_CORE_GAME_FSM_H

namespace my_auto_arena {
namespace core {

enum class GamePhase { kPrepare, kBattle, kSettlement };

enum class PhaseTransitionResult { kSuccess, kAlreadyInPhase, kIllegalTransition, kGameOver };

struct RoundOutcome {
    bool playerWon;
    int goldReward;
    int hpPenalty;
};

class GameFSM {
public:
    GameFSM();
    GameFSM(const GameFSM& other);
    GameFSM& operator=(const GameFSM& other) = default;

    GamePhase currentPhase() const;
    int currentRound() const;
    bool isGameOver() const;
    bool canPlayerAct() const;
    bool hasOutcome() const;

    PhaseTransitionResult startBattle();
    PhaseTransitionResult startSettlement(const RoundOutcome& outcome);
    PhaseTransitionResult startNextRound();

    const RoundOutcome& lastOutcome() const;
    void setGameOver();

private:
    GamePhase phase_;
    int round_;
    bool gameOver_;
    bool hasOutcome_;
    RoundOutcome lastOutcome_;
};

}  // namespace core
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_CORE_GAME_FSM_H
