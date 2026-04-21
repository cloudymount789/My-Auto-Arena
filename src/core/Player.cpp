#include "core/Player.h"

#include <algorithm>
#include <stdexcept>

namespace my_auto_arena {
namespace core {

Player::Player(int id, int gold, int hp, int level, int populationCap)
    : id_(id), gold_(gold), hp_(hp), level_(level), populationCap_(populationCap) {
    if (id < 0 || gold < 0 || hp <= 0 || level <= 0 || populationCap <= 0) {
        throw std::invalid_argument("Invalid player initial values.");
    }
}

Player::Player(const Player& other)
    : id_(other.id_),
      gold_(other.gold_),
      hp_(other.hp_),
      level_(other.level_),
      populationCap_(other.populationCap_),
      unitIds_(other.unitIds_) {}

int Player::id() const { return id_; }
int Player::gold() const { return gold_; }
int Player::hp() const { return hp_; }
int Player::level() const { return level_; }
int Player::populationCap() const { return populationCap_; }

void Player::setGold(int gold) {
    if (gold < 0) {
        throw std::invalid_argument("Gold cannot be negative.");
    }
    gold_ = gold;
}

void Player::setHp(int hp) {
    if (hp < 0) {
        throw std::invalid_argument("Hp cannot be negative.");
    }
    hp_ = hp;
}

void Player::setLevel(int level) {
    if (level <= 0) {
        throw std::invalid_argument("Level must be positive.");
    }
    level_ = level;
}

void Player::setPopulationCap(int populationCap) {
    if (populationCap <= 0) {
        throw std::invalid_argument("Population cap must be positive.");
    }
    populationCap_ = populationCap;
}

bool Player::ownsUnit(int unitId) const {
    if (unitId < 0) {
        return false;
    }
    return std::find(unitIds_.begin(), unitIds_.end(), unitId) != unitIds_.end();
}

void Player::addUnit(int unitId) {
    if (unitId < 0) {
        throw std::invalid_argument("Unit id must be non-negative.");
    }
    if (!ownsUnit(unitId)) {
        unitIds_.push_back(unitId);
    }
}

void Player::removeUnit(int unitId) {
    std::vector<int>::iterator it = std::remove(unitIds_.begin(), unitIds_.end(), unitId);
    unitIds_.erase(it, unitIds_.end());
}

int Player::unitCount() const { return static_cast<int>(unitIds_.size()); }

int Player::populationOnBoard(const Board& board) const {
    int count = 0;
    for (int row = 0; row < board.rows(); ++row) {
        for (int col = 0; col < board.cols(); ++col) {
            Position pos;
            pos.row = row;
            pos.col = col;
            const int unitId = board.occupantOnBoard(pos);
            if (ownsUnit(unitId)) {
                count += 1;
            }
        }
    }
    return count;
}

}  // namespace core
}  // namespace my_auto_arena
