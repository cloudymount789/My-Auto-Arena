#include "core/SynergySystem.h"

#include <algorithm>
#include <string>

namespace my_auto_arena {
namespace core {

namespace {

struct Tier {
    int threshold;
    int valueA;
    int valueB;
    const char* description;
};

int roundPercent(int base, int percent) {
    return static_cast<int>(base * percent / 100.0 + 0.5);
}

bool isClass(UnitClass cls, UnitClass a) {
    return cls == a;
}

bool isClass(UnitClass cls, UnitClass a, UnitClass b) {
    return cls == a || cls == b;
}

bool isClass(UnitClass cls, UnitClass a, UnitClass b, UnitClass c) {
    return cls == a || cls == b || cls == c;
}

int activeThresholdFor(int count, const Tier* tiers, int tierCount) {
    int active = 0;
    for (int i = 0; i < tierCount; ++i) {
        if (count >= tiers[i].threshold) {
            active = tiers[i].threshold;
        }
    }
    return active;
}

int nextThresholdFor(int count, const Tier* tiers, int tierCount) {
    for (int i = 0; i < tierCount; ++i) {
        if (count < tiers[i].threshold) {
            return tiers[i].threshold;
        }
    }
    return tiers[tierCount - 1].threshold;
}

const Tier* activeTierFor(int count, const Tier* tiers, int tierCount) {
    const Tier* active = nullptr;
    for (int i = 0; i < tierCount; ++i) {
        if (count >= tiers[i].threshold) {
            active = &tiers[i];
        }
    }
    return active;
}

std::string detailFromTiers(const Tier* tiers, int tierCount) {
    std::string text;
    for (int i = 0; i < tierCount; ++i) {
        if (!text.empty()) {
            text += "\n";
        }
        text += std::to_string(tiers[i].threshold) + "星: " + tiers[i].description;
    }
    return text;
}

ActiveSynergy makeSynergy(const std::string& name, int count, const Tier* tiers, int tierCount,
                          const std::string& classes, const std::string& inactiveText) {
    ActiveSynergy s;
    s.name = name;
    s.count = count;
    s.activeThreshold = activeThresholdFor(count, tiers, tierCount);
    s.nextThreshold = nextThresholdFor(count, tiers, tierCount);
    const Tier* tier = activeTierFor(count, tiers, tierCount);
    s.buffDescription = (tier == nullptr) ? inactiveText : tier->description;
    s.classesDescription = classes;
    s.detailDescription = detailFromTiers(tiers, tierCount);
    return s;
}

}  // namespace

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
            if (u->owner() == UnitOwner::player && u->unitClass() == cls) {
                count += std::max(1, std::min(3, u->starLevel()));
            }
        }
    }
    return count;
}

void SynergySystem::applyBuffs(const Board& board, std::map<int, Unit*>& units) {
    Unit::resetSynergyShieldState();
    for (std::map<int, Unit*>::iterator it = units.begin(); it != units.end(); ++it) {
        if (it->second != nullptr && it->second->owner() == UnitOwner::player) {
            it->second->clearSynergyBuffs();
        }
    }

    const int warriors = countClassOnBoard(UnitClass::kWarrior, board, units);
    const int tanks = countClassOnBoard(UnitClass::kTank, board, units);
    const int archers = countClassOnBoard(UnitClass::kArcher, board, units);
    const int mages = countClassOnBoard(UnitClass::kMage, board, units);
    const int healers = countClassOnBoard(UnitClass::kHealer, board, units);

    const int offenseStars = warriors + archers + mages;
    const int defenseStars = tanks + healers;
    const int physicalStars = tanks + warriors + archers;
    const int magicStars = mages + healers;

    int offenseAtkPercent = 0;
    if (offenseStars >= 15) offenseAtkPercent = 100;
    else if (offenseStars >= 11) offenseAtkPercent = 60;
    else if (offenseStars >= 7) offenseAtkPercent = 25;
    else if (offenseStars >= 3) offenseAtkPercent = 10;

    int defenseDefPercent = 0;
    int defenseHpPercent = 0;
    bool shieldField = false;
    if (defenseStars >= 12) {
        defenseDefPercent = 60;
        defenseHpPercent = 30;
        shieldField = true;
    } else if (defenseStars >= 9) {
        defenseDefPercent = 50;
        defenseHpPercent = 20;
    } else if (defenseStars >= 6) {
        defenseDefPercent = 20;
        defenseHpPercent = 10;
    } else if (defenseStars >= 3) {
        defenseDefPercent = 10;
    }

    int physicalAtkPercent = 0;
    int physicalDefPercent = 0;
    bool armorBreak = false;
    if (physicalStars >= 15) {
        physicalAtkPercent = 40;
        physicalDefPercent = 20;
        armorBreak = true;
    } else if (physicalStars >= 11) {
        physicalAtkPercent = 40;
        physicalDefPercent = 20;
    } else if (physicalStars >= 7) {
        physicalAtkPercent = 15;
        physicalDefPercent = 10;
    } else if (physicalStars >= 3) {
        physicalAtkPercent = 5;
        physicalDefPercent = 5;
    }

    int magicAtkPercent = 0;
    bool magicPenetration = false;
    if (magicStars >= 12) {
        magicAtkPercent = 50;
        magicPenetration = true;
    } else if (magicStars >= 9) {
        magicAtkPercent = 40;
    } else if (magicStars >= 6) {
        magicAtkPercent = 20;
    } else if (magicStars >= 3) {
        magicAtkPercent = 8;
    }

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

            const UnitClass cls = u->unitClass();
            int bonusAtk = 0;
            int bonusMagAtk = 0;
            int bonusHp = 0;
            int bonusPhysDef = 0;
            int bonusMagDef = 0;
            bool unitArmorBreak = false;
            bool unitMagicPenetration = false;
            bool unitShieldField = false;

            if (isClass(cls, UnitClass::kWarrior, UnitClass::kArcher, UnitClass::kMage)) {
                if (u->usesMagicAttack()) {
                    bonusMagAtk += roundPercent(u->baseMagicAtk(), offenseAtkPercent);
                } else {
                    bonusAtk += roundPercent(u->basePhysicalAtk(), offenseAtkPercent);
                }
            }
            if (isClass(cls, UnitClass::kTank, UnitClass::kHealer)) {
                bonusPhysDef += roundPercent(u->basePhysicalDef(), defenseDefPercent);
                bonusMagDef += roundPercent(u->baseMagicDef(), defenseDefPercent);
                bonusHp += roundPercent(u->baseMaxHp(), defenseHpPercent);
                unitShieldField = shieldField;
            }
            if (isClass(cls, UnitClass::kTank, UnitClass::kWarrior, UnitClass::kArcher)) {
                bonusAtk += roundPercent(u->basePhysicalAtk(), physicalAtkPercent);
                bonusPhysDef += roundPercent(u->basePhysicalDef(), physicalDefPercent);
                unitArmorBreak = armorBreak;
            }
            if (isClass(cls, UnitClass::kMage, UnitClass::kHealer)) {
                bonusMagAtk += roundPercent(u->baseMagicAtk(), magicAtkPercent);
                unitMagicPenetration = magicPenetration;
            }

            u->setSynergyBuffs(bonusAtk, bonusMagAtk, bonusHp, bonusPhysDef, bonusMagDef,
                               unitArmorBreak, unitMagicPenetration, unitShieldField);
        }
    }
}

void SynergySystem::clearBuffs(std::vector<Unit*>& playerUnits) {
    Unit::resetSynergyShieldState();
    for (std::size_t i = 0; i < playerUnits.size(); ++i) {
        if (playerUnits.at(i) != nullptr) {
            playerUnits.at(i)->clearSynergyBuffs();
        }
    }
}

std::vector<ActiveSynergy> SynergySystem::getActiveSynergies(const Board& board,
                                                              const std::map<int, Unit*>& units) {
    static const Tier kOffenseTiers[4] = {
        {3, 10, 0, "战士/射手/法师各自 +10% 攻击"},
        {7, 25, 0, "战士/射手/法师各自 +25% 攻击"},
        {11, 60, 0, "战士/射手/法师各自 +60% 攻击"},
        {15, 100, 0, "战士/射手/法师各自 +100% 攻击"}
    };
    static const Tier kDefenseTiers[4] = {
        {3, 10, 0, "重甲战士/治疗师 +10% 物防与法防"},
        {6, 20, 10, "重甲战士/治疗师 +20% 物防与法防，+10% 生命"},
        {9, 50, 20, "重甲战士/治疗师 +50% 物防与法防，+20% 生命"},
        {12, 60, 30, "重甲战士/治疗师 +60% 物防与法防，+30% 生命；受击4tick后全队免伤1tick"}
    };
    static const Tier kPhysicalTiers[4] = {
        {3, 5, 5, "重甲战士/战士/射手 +5% 物攻，+5% 物防"},
        {7, 15, 10, "重甲战士/战士/射手 +15% 物攻，+10% 物防"},
        {11, 40, 20, "重甲战士/战士/射手 +40% 物攻，+20% 物防"},
        {15, 40, 20, "保留11星增益，并获得破甲：物理伤害忽略30%防御"}
    };
    static const Tier kMagicTiers[4] = {
        {3, 8, 0, "法师/治疗师 +8% 法攻"},
        {6, 20, 0, "法师/治疗师 +20% 法攻"},
        {9, 40, 0, "法师/治疗师 +40% 法攻"},
        {12, 50, 0, "法师/治疗师 +50% 法攻；穿透：法术伤害忽略敌方法防"}
    };

    const int warriors = countClassOnBoard(UnitClass::kWarrior, board, units);
    const int tanks = countClassOnBoard(UnitClass::kTank, board, units);
    const int archers = countClassOnBoard(UnitClass::kArcher, board, units);
    const int mages = countClassOnBoard(UnitClass::kMage, board, units);
    const int healers = countClassOnBoard(UnitClass::kHealer, board, units);

    std::vector<ActiveSynergy> result;
    result.push_back(makeSynergy("进攻就是最好的防守！", warriors + archers + mages,
                                 kOffenseTiers, 4, "战士 / 射手 / 法师", "未激活"));
    result.push_back(makeSynergy("因为太怕痛就全点防御力了", tanks + healers,
                                 kDefenseTiers, 4, "重甲战士 / 治疗师", "未激活"));
    result.push_back(makeSynergy("轻轻敲醒沉睡的心灵", tanks + warriors + archers,
                                 kPhysicalTiers, 4, "重甲战士 / 战士 / 射手", "未激活"));
    result.push_back(makeSynergy("要用魔法打败魔法", mages + healers,
                                 kMagicTiers, 4, "法师 / 治疗师", "未激活"));
    return result;
}

}  // namespace core
}  // namespace my_auto_arena
