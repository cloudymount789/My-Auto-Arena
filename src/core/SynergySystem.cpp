#include "core/SynergySystem.h"

namespace my_auto_arena {
namespace core {

int SynergySystem::countClassOnBoard(UnitClass cls, const Board& board,
                                      const std::map<int, Unit*>& units) {
    int count = 0;
    for (int row = 0; row < board.rows(); ++row) {
        for (int col = 0; col < board.cols(); ++col) {
            const int occ = board.occupantOnBoard(Position{row, col});
            if (occ == Board::kEmptySlot) {
                continue;
            }
            std::map<int, Unit*>::const_iterator it = units.find(occ);
            if (it == units.end() || it->second == nullptr) {
                continue;
            }
            const Unit* u = it->second;
            // 仅统计玩家阵营且职业匹配的单位。
            if (u->owner() == UnitOwner::player && u->unitClass() == cls) {
                ++count;
            }
        }
    }
    return count;
}

void SynergySystem::applyBuffs(const Board& board, std::map<int, Unit*>& units) {
    // 统计各职业在棋盘上的玩家单位数量。
    const int warriors = countClassOnBoard(UnitClass::kWarrior, board, units);
    const int tanks    = countClassOnBoard(UnitClass::kTank,    board, units);
    const int archers  = countClassOnBoard(UnitClass::kArcher,  board, units);
    const int mages    = countClassOnBoard(UnitClass::kMage,    board, units);
    const int healers  = countClassOnBoard(UnitClass::kHealer,  board, units);

    // 近战羁绊（战士+坦克总数）：2→所有玩家+300 HP；4→+700 HP。
    const int meleeCount = warriors + tanks;
    int meleeHpBonus = 0;
    if (meleeCount >= 4)      meleeHpBonus = 700;
    else if (meleeCount >= 2) meleeHpBonus = 300;

    // 弓手羁绊（射手数量）：2→弓手+50 ATK；3→弓手+120 ATK。
    int archerAtkBonus = 0;
    if (archers >= 3)      archerAtkBonus = 120;
    else if (archers >= 2) archerAtkBonus = 50;

    // 法术羁绊（法师数量）：1→法师+70 ATK；2→法师+160 ATK。
    int mageAtkBonus = 0;
    if (mages >= 2)      mageAtkBonus = 160;
    else if (mages >= 1) mageAtkBonus = 70;

    // 圣愈羁绊（治疗师数量）：1→所有玩家+400 HP；2→+900 HP。
    int healHpBonus = 0;
    if (healers >= 2)      healHpBonus = 900;
    else if (healers >= 1) healHpBonus = 400;

    // 遍历棋盘上的玩家单位并设置对应羁绊 BUFF。
    for (int row = 0; row < board.rows(); ++row) {
        for (int col = 0; col < board.cols(); ++col) {
            const int occ = board.occupantOnBoard(Position{row, col});
            if (occ == Board::kEmptySlot) {
                continue;
            }
            std::map<int, Unit*>::iterator it = units.find(occ);
            if (it == units.end() || it->second == nullptr) {
                continue;
            }
            Unit* u = it->second;
            if (u->owner() != UnitOwner::player) {
                continue;
            }

            int bonusAtk = 0;
            int bonusHp  = meleeHpBonus + healHpBonus;  // 全体加成叠加

            if (u->unitClass() == UnitClass::kArcher) {
                bonusAtk = archerAtkBonus;
            } else if (u->unitClass() == UnitClass::kMage) {
                bonusAtk = mageAtkBonus;
            }

            u->setSynergyBuffs(bonusAtk, bonusHp);
        }
    }
}

void SynergySystem::clearBuffs(std::vector<Unit*>& playerUnits) {
    for (std::size_t i = 0; i < playerUnits.size(); ++i) {
        if (playerUnits.at(i) != nullptr) {
            playerUnits.at(i)->clearSynergyBuffs();
        }
    }
}

std::vector<ActiveSynergy> SynergySystem::getActiveSynergies(const Board& board,
                                                               const std::map<int, Unit*>& units) {
    std::vector<ActiveSynergy> result;

    const int warriors = countClassOnBoard(UnitClass::kWarrior, board, units);
    const int tanks    = countClassOnBoard(UnitClass::kTank,    board, units);
    const int archers  = countClassOnBoard(UnitClass::kArcher,  board, units);
    const int mages    = countClassOnBoard(UnitClass::kMage,    board, units);
    const int healers  = countClassOnBoard(UnitClass::kHealer,  board, units);
    const int melee    = warriors + tanks;

    // 近战羁绊
    {
        ActiveSynergy s;
        s.name = "近战";
        s.count = melee;
        if (melee >= 4) {
            s.activeThreshold = 4;
            s.buffDescription = "所有单位 +700 HP";
        } else if (melee >= 2) {
            s.activeThreshold = 2;
            s.buffDescription = "所有单位 +300 HP";
        } else {
            s.activeThreshold = 0;
            s.buffDescription = "2/4 激活";
        }
        result.push_back(s);
    }

    // 弓手羁绊
    {
        ActiveSynergy s;
        s.name = "弓手";
        s.count = archers;
        if (archers >= 3) {
            s.activeThreshold = 3;
            s.buffDescription = "弓手 +120 ATK";
        } else if (archers >= 2) {
            s.activeThreshold = 2;
            s.buffDescription = "弓手 +50 ATK";
        } else {
            s.activeThreshold = 0;
            s.buffDescription = "2/3 激活";
        }
        result.push_back(s);
    }

    // 法术羁绊
    {
        ActiveSynergy s;
        s.name = "法术";
        s.count = mages;
        if (mages >= 2) {
            s.activeThreshold = 2;
            s.buffDescription = "法师 +160 ATK";
        } else if (mages >= 1) {
            s.activeThreshold = 1;
            s.buffDescription = "法师 +70 ATK";
        } else {
            s.activeThreshold = 0;
            s.buffDescription = "1/2 激活";
        }
        result.push_back(s);
    }

    // 圣愈羁绊
    {
        ActiveSynergy s;
        s.name = "圣愈";
        s.count = healers;
        if (healers >= 2) {
            s.activeThreshold = 2;
            s.buffDescription = "所有单位 +900 HP";
        } else if (healers >= 1) {
            s.activeThreshold = 1;
            s.buffDescription = "所有单位 +400 HP";
        } else {
            s.activeThreshold = 0;
            s.buffDescription = "1/2 激活";
        }
        result.push_back(s);
    }

    return result;
}

}  // namespace core
}  // namespace my_auto_arena
