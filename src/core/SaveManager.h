#ifndef MY_AUTO_ARENA_CORE_SAVE_MANAGER_H
#define MY_AUTO_ARENA_CORE_SAVE_MANAGER_H

#include <map>
#include <string>
#include <vector>

#include "core/Board.h"
#include "core/GameFSM.h"
#include "core/Item.h"
#include "core/Player.h"
#include "core/Unit.h"

namespace my_auto_arena {
namespace core {

struct DeploymentEntry {
    int unitId;
    Position position;
};

// 存档管理器：将游戏状态序列化为文本文件（key=value 格式），支持存档与读档。
class SaveManager {
public:
    // 将当前游戏状态写入文件；成功返回 true，失败返回 false。
    static bool save(const std::string& filepath,
                     const GameFSM& fsm,
                     const Player& player,
                     const Board& board,
                     const std::vector<Unit*>& playerUnits,
                     const std::vector<ItemType>& pendingItems,
                     const std::vector<DeploymentEntry>* savedDeployment = nullptr);

    // 从文件读取并恢复游戏状态；成功返回 true，失败返回 false。
    static bool load(const std::string& filepath,
                     GameFSM& fsm,
                     Player& player,
                     Board& board,
                     std::vector<Unit*>& playerUnits,
                     std::map<int, Unit*>& unitsMap,
                     std::vector<ItemType>& pendingItems,
                     std::vector<DeploymentEntry>* savedDeployment = nullptr);

private:
    // 将职业枚举转换为字符串（用于序列化）。
    static std::string unitClassToStr(UnitClass cls);
    static UnitClass strToUnitClass(const std::string& s);

    // 将装备枚举转换为字符串（用于序列化）。
    static std::string itemTypeToStr(ItemType t);
    static ItemType strToItemType(const std::string& s);
};

}  // namespace core
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_CORE_SAVE_MANAGER_H
