#include "core/SaveManager.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

#include "core/HeroUnits.h"

namespace my_auto_arena {
namespace core {

// ── 辅助转换函数 ─────────────────────────────────────────────────────────────

std::string SaveManager::unitClassToStr(UnitClass cls) {
    switch (cls) {
        case UnitClass::kWarrior: return "warrior";
        case UnitClass::kArcher:  return "archer";
        case UnitClass::kTank:    return "tank";
        case UnitClass::kMage:    return "mage";
        case UnitClass::kHealer:  return "healer";
        default:                  return "none";
    }
}

UnitClass SaveManager::strToUnitClass(const std::string& s) {
    if (s == "warrior") return UnitClass::kWarrior;
    if (s == "archer")  return UnitClass::kArcher;
    if (s == "tank")    return UnitClass::kTank;
    if (s == "mage")    return UnitClass::kMage;
    if (s == "healer")  return UnitClass::kHealer;
    return UnitClass::kNone;
}

std::string SaveManager::itemTypeToStr(ItemType t) {
    switch (t) {
        case ItemType::kSword:    return "sword";
        case ItemType::kArmor:    return "armor";
        case ItemType::kRing:     return "ring";
        case ItemType::kTalisman: return "talisman";
        default:                  return "none";
    }
}

ItemType SaveManager::strToItemType(const std::string& s) {
    if (s == "sword")    return ItemType::kSword;
    if (s == "armor")    return ItemType::kArmor;
    if (s == "ring")     return ItemType::kRing;
    if (s == "talisman") return ItemType::kTalisman;
    return ItemType::kNone;
}

// ── 存档 ─────────────────────────────────────────────────────────────────────

bool SaveManager::save(const std::string& filepath,
                       const GameFSM& fsm,
                       const Player& player,
                       const Board& board,
                       const std::vector<Unit*>& playerUnits,
                       const std::vector<ItemType>& pendingItems) {
    try {
        std::ofstream ofs(filepath);
        if (!ofs.is_open()) {
            return false;
        }

        ofs << "# Synera Auto-Arena 存档\n";
        ofs << "round=" << fsm.currentRound() << "\n";
        ofs << "player_hp=" << player.hp() << "\n";
        ofs << "player_gold=" << player.gold() << "\n";
        ofs << "player_pop_cap=" << player.populationCap() << "\n";
        ofs << "unit_count=" << playerUnits.size() << "\n";

        // 序列化每个玩家英雄的状态。
        for (std::size_t i = 0; i < playerUnits.size(); ++i) {
            const Unit* u = playerUnits.at(i);
            if (u == nullptr) continue;

            const std::string prefix = "unit" + std::to_string(i) + "_";
            ofs << prefix << "id=" << u->id() << "\n";
            ofs << prefix << "name=" << u->name() << "\n";
            ofs << prefix << "class=" << unitClassToStr(u->unitClass()) << "\n";
            ofs << prefix << "star=" << u->starLevel() << "\n";
            ofs << prefix << "item=" << itemTypeToStr(u->equippedItem()) << "\n";

            // 记录单位在棋盘或备战区的位置。
            const Position pos = board.findUnitOnBoard(u->id());
            if (board.inBounds(pos)) {
                ofs << prefix << "loc=board\n";
                ofs << prefix << "row=" << pos.row << "\n";
                ofs << prefix << "col=" << pos.col << "\n";
            } else {
                int benchSlot = -1;
                for (int slot = 0; slot < board.benchSize(); ++slot) {
                    if (board.occupantOnBench(slot) == u->id()) {
                        benchSlot = slot;
                        break;
                    }
                }
                ofs << prefix << "loc=bench\n";
                ofs << prefix << "bench_slot=" << benchSlot << "\n";
            }
        }

        // 序列化待装备道具列表。
        ofs << "pending_item_count=" << pendingItems.size() << "\n";
        for (std::size_t i = 0; i < pendingItems.size(); ++i) {
            ofs << "pending_item" << i << "=" << itemTypeToStr(pendingItems.at(i)) << "\n";
        }

        ofs.close();
        return true;
    } catch (...) {
        return false;
    }
}

// ── 读档 ─────────────────────────────────────────────────────────────────────

bool SaveManager::load(const std::string& filepath,
                       GameFSM& fsm,
                       Player& player,
                       Board& board,
                       std::vector<Unit*>& playerUnits,
                       std::map<int, Unit*>& unitsMap,
                       std::vector<ItemType>& pendingItems) {
    try {
        std::ifstream ifs(filepath);
        if (!ifs.is_open()) {
            return false;
        }

        // 将文件内容解析为 key→value 映射。
        std::map<std::string, std::string> kv;
        std::string line;
        while (std::getline(ifs, line)) {
            if (line.empty() || line[0] == '#') continue;
            const std::string::size_type eq = line.find('=');
            if (eq == std::string::npos) continue;
            kv[line.substr(0, eq)] = line.substr(eq + 1);
        }
        ifs.close();

        // 恢复 FSM 轮次（只重置到 kPrepare 并前进到目标轮）。
        const int round = std::stoi(kv.at("round"));
        while (fsm.currentRound() < round) {
            fsm.startBattle();
            RoundOutcome dummy;
            dummy.playerWon = true;
            dummy.goldReward = 0;
            dummy.hpPenalty = 0;
            dummy.gameOver = false;
            fsm.startSettlement(dummy);
            fsm.startNextRound();
        }

        // 恢复玩家属性。
        player.setHp(std::stoi(kv.at("player_hp")));
        player.setGold(std::stoi(kv.at("player_gold")));
        player.setPopulationCap(std::stoi(kv.at("player_pop_cap")));

        // 清理旧玩家单位。
        for (std::size_t i = 0; i < playerUnits.size(); ++i) {
            if (playerUnits.at(i) != nullptr) {
                unitsMap.erase(playerUnits.at(i)->id());
                player.removeUnit(playerUnits.at(i)->id());
                delete playerUnits.at(i);
            }
        }
        playerUnits.clear();

        // 清空棋盘格子与备战区：旧单位的占位 ID 仍残留在 Board 中；
        // 若不清除，placeOnBoard/placeOnBench 会因格子已占用而静默失败。
        for (int row = 0; row < board.rows(); ++row) {
            for (int col = 0; col < board.cols(); ++col) {
                board.clearOnBoard(Position{row, col});
            }
        }
        for (int slot = 0; slot < board.benchSize(); ++slot) {
            board.clearOnBench(slot);
        }

        // 重建玩家单位。
        const int unitCount = std::stoi(kv.at("unit_count"));
        for (int i = 0; i < unitCount; ++i) {
            const std::string prefix = "unit" + std::to_string(i) + "_";
            const int uid    = std::stoi(kv.at(prefix + "id"));
            const std::string uname = kv.at(prefix + "name");
            const UnitClass ucls  = strToUnitClass(kv.at(prefix + "class"));
            const int star   = std::stoi(kv.at(prefix + "star"));
            const ItemType item = strToItemType(kv.at(prefix + "item"));

            // 根据名称/职业还原对应英雄类型。
            HeroType htype = HeroType::kWarrior;
            if (ucls == UnitClass::kArcher)  htype = HeroType::kArcher;
            else if (ucls == UnitClass::kTank)    htype = HeroType::kTank;
            else if (ucls == UnitClass::kMage)    htype = HeroType::kMage;
            else if (ucls == UnitClass::kHealer)  htype = HeroType::kHealer;
            (void)uname;

            Unit* u = createHero(htype, uid, UnitOwner::player);
            if (star >= 2) u->upgradeToStar(2);
            if (star >= 3) u->upgradeToStar(3);
            if (item != ItemType::kNone) u->equipItem(item);

            playerUnits.push_back(u);
            unitsMap[uid] = u;
            player.addUnit(uid);

            // 还原棋盘/备战区位置。
            const std::string loc = kv.at(prefix + "loc");
            if (loc == "board") {
                const int row = std::stoi(kv.at(prefix + "row"));
                const int col = std::stoi(kv.at(prefix + "col"));
                board.placeOnBoard(uid, Position{row, col});
            } else {
                const int slot = std::stoi(kv.at(prefix + "bench_slot"));
                if (slot >= 0) {
                    board.placeOnBench(uid, slot);
                }
            }
        }

        // 恢复待装备道具列表。
        pendingItems.clear();
        const int pendingCount = std::stoi(kv.at("pending_item_count"));
        for (int i = 0; i < pendingCount; ++i) {
            const std::string key = "pending_item" + std::to_string(i);
            pendingItems.push_back(strToItemType(kv.at(key)));
        }

        return true;
    } catch (...) {
        return false;
    }
}

}  // namespace core
}  // namespace my_auto_arena
