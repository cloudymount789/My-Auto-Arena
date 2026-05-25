#include "core/SynergySystem.h"

#include <string>

namespace my_auto_arena {
namespace core {

int SynergySystem::countClassOnBoard(UnitClass cls, const Board& board,
                                      const std::map<int, Unit*>& units) {
    // 按星级加权统计：★1=1点，★2=2点，★3=3点。
    // 例：3个★1=3点(T1激活); 1个★3+1个★1=4点(仍T1); 3个★2=6点(T2激活)。
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
            if (u->owner() == UnitOwner::player && u->unitClass() == cls) {
                // 升星规则：3×★1→★2，3×★2→★3
                // 所以 ★2 等效于 3 个★1（3点），★3 等效于 9 个★1（9点）
                const int starLevel = u->starLevel();
                const int pts = (starLevel == 3) ? 9 : (starLevel == 2) ? 3 : 1;
                count += pts;
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

    // ─────────────────────────────────────────────────────────────────
    // 星级加权羁绊规则（点数：★1=1pt，★2=3pt，★3=9pt）：
    //   T1 激活阈值 = 3  → 3个★1，或 1个★2，均等效
    //   T2 激活阈值 = 9  → 3个★2，或 1个★3，均等效
    // 更高阈值对应更大奖励，鼓励专攻路线并深度投资升星。
    // ─────────────────────────────────────────────────────────────────

    // 近战羁绊（战士+重甲战士）：T1(3)→ +70 ATK；T2(9)→ +180 ATK +1000 HP。
    const int meleeCount = warriors + tanks;
    int meleeAtkBonus = 0;
    int meleeHpBonus  = 0;
    if (meleeCount >= 9) {
        meleeAtkBonus = 180;
        meleeHpBonus  = 1000;
    } else if (meleeCount >= 3) {
        meleeAtkBonus = 70;
    }

    // 弓手羁绊（射手）：T1(3)→ +160 ATK；T2(9)→ +400 ATK。
    int archerAtkBonus = 0;
    if (archers >= 9)      archerAtkBonus = 400;
    else if (archers >= 3) archerAtkBonus = 160;

    // 法术羁绊（法师）：T1(3)→ +180 ATK；T2(9)→ +450 ATK。
    int mageAtkBonus = 0;
    if (mages >= 9)      mageAtkBonus = 450;
    else if (mages >= 3) mageAtkBonus = 180;

    // 圣愈羁绊（治疗师）：T1(3)→全体 +1000 HP；T2(9)→全体 +2500 HP。
    int healHpBonus = 0;
    if (healers >= 9)      healHpBonus = 2500;
    else if (healers >= 3) healHpBonus = 1000;

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

            int bonusAtk    = 0;
            int bonusMagAtk = 0;
            // 圣愈为全体提供 HP；近战羁绊只给近战单位加 ATK 和 HP。
            int bonusHp     = healHpBonus;

            if (u->unitClass() == UnitClass::kWarrior || u->unitClass() == UnitClass::kTank) {
                bonusAtk = meleeAtkBonus;
                bonusHp += meleeHpBonus;
            } else if (u->unitClass() == UnitClass::kArcher) {
                bonusAtk = archerAtkBonus;
            } else if (u->unitClass() == UnitClass::kMage) {
                // 法术羁绊加成走 magicAtk 通道，不污染物理攻击。
                bonusMagAtk = mageAtkBonus;
            }

            u->setSynergyBuffs(bonusAtk, bonusMagAtk, bonusHp);
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

    // 近战羁绊（★1=1pt，★2=3pt，★3=9pt；T1=3, T2=9）
    {
        ActiveSynergy s;
        s.name = "近战";
        s.count = melee;
        if (melee >= 9) {
            s.activeThreshold = 9;
            s.buffDescription = "近战 +180 ATK +1000 HP";
        } else if (melee >= 3) {
            s.activeThreshold = 3;
            s.buffDescription = "近战 +70 ATK";
        } else {
            s.activeThreshold = 0;
            s.buffDescription = "3/9 激活 (已有" + std::to_string(melee) + "点)";
        }
        result.push_back(s);
    }

    // 弓手羁绊（T1=3, T2=9）
    {
        ActiveSynergy s;
        s.name = "弓手";
        s.count = archers;
        if (archers >= 9) {
            s.activeThreshold = 9;
            s.buffDescription = "弓手 +400 ATK";
        } else if (archers >= 3) {
            s.activeThreshold = 3;
            s.buffDescription = "弓手 +160 ATK";
        } else {
            s.activeThreshold = 0;
            s.buffDescription = "3/9 激活 (已有" + std::to_string(archers) + "点)";
        }
        result.push_back(s);
    }

    // 法术羁绊（T1=3, T2=9）
    {
        ActiveSynergy s;
        s.name = "法术";
        s.count = mages;
        if (mages >= 9) {
            s.activeThreshold = 9;
            s.buffDescription = "法师 +450 ATK";
        } else if (mages >= 3) {
            s.activeThreshold = 3;
            s.buffDescription = "法师 +180 ATK";
        } else {
            s.activeThreshold = 0;
            s.buffDescription = "3/9 激活 (已有" + std::to_string(mages) + "点)";
        }
        result.push_back(s);
    }

    // 圣愈羁绊（T1=3, T2=9）
    {
        ActiveSynergy s;
        s.name = "圣愈";
        s.count = healers;
        if (healers >= 9) {
            s.activeThreshold = 9;
            s.buffDescription = "全体 +2500 HP";
        } else if (healers >= 3) {
            s.activeThreshold = 3;
            s.buffDescription = "全体 +1000 HP";
        } else {
            s.activeThreshold = 0;
            s.buffDescription = "3/9 激活 (已有" + std::to_string(healers) + "点)";
        }
        result.push_back(s);
    }

    return result;
}

}  // namespace core
}  // namespace my_auto_arena
