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

    // ──────────────────────────────────────────────────────────────────
    // 羁绊设计说明（策略性层次）：
    //  · 近战：战士+重甲战士总数  → 近战单位获得大量 ATK 加成，打造极强前排
    //  · 弓手：射手数量          → 射手获得海量 ATK，全力堆射手可形成超高单体输出
    //  · 法术：法师数量          → 法师 ATK 大幅提升，法师技能爆发质变
    //  · 圣愈：治疗师数量        → 全体玩家英雄获得大量 HP，适合持久战/抗高伤阵容
    // 策略目的：以上4种羁绊回报差异显著，迫使玩家专攻路线而非随意混搭。
    // ──────────────────────────────────────────────────────────────────

    // 近战羁绊（战士+坦克）：2→近战单位 +45 ATK；4→近战单位 +110 ATK + 500 HP。
    const int meleeCount = warriors + tanks;
    int meleeAtkBonus = 0;
    int meleeHpBonus  = 0;
    if (meleeCount >= 4) {
        meleeAtkBonus = 110;
        meleeHpBonus  = 500;
    } else if (meleeCount >= 2) {
        meleeAtkBonus = 45;
    }

    // 弓手羁绊（射手）：2→射手 +100 ATK；3→射手 +260 ATK。
    int archerAtkBonus = 0;
    if (archers >= 3)      archerAtkBonus = 260;
    else if (archers >= 2) archerAtkBonus = 100;

    // 法术羁绊（法师）：1→法师 +120 ATK；2→法师 +300 ATK。
    int mageAtkBonus = 0;
    if (mages >= 2)      mageAtkBonus = 300;
    else if (mages >= 1) mageAtkBonus = 120;

    // 圣愈羁绊（治疗师）：1→全体玩家 +800 HP；2→全体玩家 +2000 HP。
    int healHpBonus = 0;
    if (healers >= 2)      healHpBonus = 2000;
    else if (healers >= 1) healHpBonus = 800;

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
            // 圣愈为全体提供 HP；近战羁绊只给近战单位加 ATK 和 HP。
            int bonusHp  = healHpBonus;

            if (u->unitClass() == UnitClass::kWarrior || u->unitClass() == UnitClass::kTank) {
                bonusAtk = meleeAtkBonus;
                bonusHp += meleeHpBonus;
            } else if (u->unitClass() == UnitClass::kArcher) {
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
            s.buffDescription = "近战 +110 ATK +500 HP";
        } else if (melee >= 2) {
            s.activeThreshold = 2;
            s.buffDescription = "近战 +45 ATK";
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
            s.buffDescription = "弓手 +260 ATK";
        } else if (archers >= 2) {
            s.activeThreshold = 2;
            s.buffDescription = "弓手 +100 ATK";
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
            s.buffDescription = "法师 +300 ATK";
        } else if (mages >= 1) {
            s.activeThreshold = 1;
            s.buffDescription = "法师 +120 ATK";
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
            s.buffDescription = "全体 +2000 HP";
        } else if (healers >= 1) {
            s.activeThreshold = 1;
            s.buffDescription = "全体 +800 HP";
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
