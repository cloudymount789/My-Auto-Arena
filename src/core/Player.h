#ifndef MY_AUTO_ARENA_CORE_PLAYER_H
#define MY_AUTO_ARENA_CORE_PLAYER_H

#include <vector>

#include "core/Board.h"

namespace my_auto_arena {
namespace core {

class Player {
public:
    // 玩家全局实体：管理血量、金币、等级、人口上限与拥有单位集合。
    Player(int id, int gold, int hp, int level, int populationCap);
    Player(const Player& other);

    int id() const;
    int gold() const;
    int hp() const;
    int level() const;
    int populationCap() const;

    void setGold(int gold);
    void setHp(int hp);
    void setLevel(int level);
    void setPopulationCap(int populationCap);

    bool ownsUnit(int unitId) const;
    void addUnit(int unitId);
    void removeUnit(int unitId);
    int unitCount() const;

    int populationOnBoard(const Board& board) const;

private:
    int id_;
    int gold_;
    int hp_;
    int level_;
    int populationCap_;
    std::vector<int> unitIds_;
};

}  // namespace core
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_CORE_PLAYER_H
