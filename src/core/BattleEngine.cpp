#include "core/BattleEngine.h"

#include <climits>

#include "core/Pathfinder.h"

namespace my_auto_arena {
namespace core {

namespace {

// 流程：根据施法者职业选择 GUI 技能特效类型 ──> 未覆盖职业使用默认弹体特效
BattleEvent::SkillVfxType skillVfxTypeForClass(UnitClass cls) {
    switch (cls) {
        case UnitClass::kWarrior: return BattleEvent::SkillVfxType::kStunSingle;
        case UnitClass::kArcher:  return BattleEvent::SkillVfxType::kLineAoe;
        case UnitClass::kTank:    return BattleEvent::SkillVfxType::kAdjacentAoe;
        case UnitClass::kMage:    return BattleEvent::SkillVfxType::kRangeAoe;
        case UnitClass::kHealer:  return BattleEvent::SkillVfxType::kHeal;
        default:                  return BattleEvent::SkillVfxType::kNone;
    }
}

}  // namespace

BattleEngine::BattleEngine(Board& board, std::map<int, Unit*>& units)
    : board_(board), units_(units), tickCount_(0), finished_(false), outcome_{false, 0, 0, false}, defeatHpPenalty_(4) {}

void BattleEngine::setDefeatHpPenalty(int hpPenalty) { defeatHpPenalty_ = hpPenalty; }

const std::vector<BattleEvent>& BattleEngine::lastTickEvents() const { return tickEvents_; }

// 流程：已结束则跳过 ──> 重置事件/单位状态 ──> 阶段一（技能/普攻）
//       ──> 清理死亡并判胜 ──> 阶段二（向目标移动一格）──> 再次清理/判胜 ──> 超时判负
void BattleEngine::tick() {
    if (finished_) {
        return;
    }

    ++tickCount_;
    Unit::beginSynergyDamageTick(tickCount_);
    tickEvents_.clear();  // 每 tick 重置事件列表

    // 每 tick 开始时将所有存活单位重置为空闲（kIdle），随后按行为更新到对应状态。
    for (std::map<int, Unit*>::iterator it = units_.begin(); it != units_.end(); ++it) {
        if (it->second != nullptr && it->second->isAlive()) {
            it->second->setState(UnitState::kIdle);
        }
    }

    // 阶段一：仅处理普攻与满蓝技能。若移动与攻击同一轮结算，map 中 id 较小的单位
    // 会先移动到攻击范围并抢先出手，导致贴近的单位被白打；拆成两阶段可保证接战对称。
    for (std::map<int, Unit*>::iterator it = units_.begin(); it != units_.end(); ++it) {
        Unit* attacker = it->second;
        if (attacker == nullptr || !attacker->isAlive()) {
            continue;
        }
        if (attacker->isStunned()) {
            attacker->tickStun();
            continue;
        }
        Unit* target = selectTarget(*attacker);
        if (target == nullptr) {
            continue;
        }
        const Position attackerPos = board_.findUnitOnBoard(attacker->id());
        const Position targetPos = board_.findUnitOnBoard(target->id());

        if (attacker->mana() >= attacker->maxMana()) {
            attacker->setState(UnitState::kCasting);
            // 记录技能施法事件（在技能执行前记录，保证坐标有效）。
            BattleEvent ev;
            ev.type        = BattleEvent::Type::kSkill;
            ev.sourceId    = attacker->id();
            ev.targetId    = target->id();
            ev.isMelee     = false;
            ev.sourceClass = attacker->unitClass();
            ev.sourceOwner = attacker->owner();
            ev.srcRow      = attackerPos.row;  ev.srcCol = attackerPos.col;
            ev.tgtRow      = targetPos.row;    ev.tgtCol = targetPos.col;
            ev.skillVfxType = skillVfxTypeForClass(attacker->unitClass());
            if (attacker->unitClass() == UnitClass::kArcher && board_.inBounds(attackerPos) &&
                board_.inBounds(targetPos)) {
                const int dr = attackerPos.row - targetPos.row;
                const int dc = attackerPos.col - targetPos.col;
                ev.lineIsVertical = (dr * dr >= dc * dc);
            } else {
                ev.lineIsVertical = false;
            }
            tickEvents_.push_back(ev);
            attacker->castFullManaSkill(board_, units_, target);
            continue;
        }

        if (inRange(*attacker, attackerPos, targetPos)) {
            tryNormalAttack(attacker, target, attackerPos, targetPos);
        }
    }

    clearDeadUnits();
    resolveEndState();
    if (finished_) {
        Unit::endSynergyDamageTick();
        return;
    }

    // 阶段二：仅处理仍不在攻击范围内的单位移动（满蓝施法已在阶段一完成）。
    for (std::map<int, Unit*>::iterator it = units_.begin(); it != units_.end(); ++it) {
        Unit* attacker = it->second;
        if (attacker == nullptr || !attacker->isAlive()) {
            continue;
        }
        if (attacker->isStunned()) {
            attacker->tickStun();
            continue;
        }
        Unit* target = selectTarget(*attacker);
        if (target == nullptr) {
            continue;
        }
        const Position attackerPos = board_.findUnitOnBoard(attacker->id());
        const Position targetPos = board_.findUnitOnBoard(target->id());
        if (inRange(*attacker, attackerPos, targetPos)) {
            continue;
        }
        attacker->setState(UnitState::kMoving);
        tryMoveTowardTarget(attacker, target);
    }

    clearDeadUnits();
    resolveEndState();

    if (!finished_ && tickCount_ >= kMaxTicks) {
        finished_ = true;
        outcome_.playerWon = false;
        outcome_.goldReward = 0;
        outcome_.hpPenalty = defeatHpPenalty_;
        outcome_.gameOver = false;  // 由 PvERoundRunner 在扣血后判断并设置
    }
    Unit::endSynergyDamageTick();
}

bool BattleEngine::isFinished() const { return finished_; }

// 流程：检查攻击者/目标有效性与距离 ──> 按攻击类型结算伤害 ──> 回蓝并记录攻击事件
bool BattleEngine::tryNormalAttack(Unit* attacker, Unit* target, Position attackerPos, Position targetPos) {
    if (attacker == nullptr || target == nullptr || !attacker->isAlive() || !target->isAlive()) {
        return false;
    }

    double& gauge = attackGauge_[attacker->id()];
    gauge += static_cast<double>(attacker->attackSpeed()) / 100.0;
    if (gauge < 1.0) {
        return false;
    }
    gauge -= 1.0;

    attacker->setState(UnitState::kAttacking);
    BattleEvent ev;
    ev.type        = BattleEvent::Type::kAttack;
    ev.sourceId    = attacker->id();
    ev.targetId    = target->id();
    ev.isMelee     = (attacker->attackRange() <= 1);
    ev.sourceClass = attacker->unitClass();
    ev.sourceOwner = attacker->owner();
    ev.srcRow      = attackerPos.row;
    ev.srcCol      = attackerPos.col;
    ev.tgtRow      = targetPos.row;
    ev.tgtCol      = targetPos.col;
    ev.skillVfxType = BattleEvent::SkillVfxType::kNone;
    ev.lineIsVertical = false;
    tickEvents_.push_back(ev);

    if (attacker->usesMagicAttack()) {
        target->takeMagicDamage(attacker->magicAtk(), attacker->magicDefenseIgnorePercent());
    } else {
        target->takePhysicalDamage(attacker->physicalAtk(), attacker->physicalDefenseIgnorePercent());
    }
    attacker->gainMana(kManaPerAttack);
    return true;
}

int BattleEngine::tickCount() const { return tickCount_; }

RoundOutcome BattleEngine::outcome() const { return outcome_; }

// 流程：取攻击者坐标 ──> 遍历敌方存活单位 ──> 按距离优先、同距低血优先、再按坐标打破平局
//       ──> 返回最佳目标（无合法目标则 nullptr）
Unit* BattleEngine::selectTarget(const Unit& attacker) const {
    const Position attackerPos = board_.findUnitOnBoard(attacker.id());
    if (!board_.inBounds(attackerPos)) {
        return nullptr;
    }

    Unit* best = nullptr;
    int bestDist = INT_MAX;
    Position bestPos{-1, -1};
    for (std::map<int, Unit*>::const_iterator cit = units_.begin(); cit != units_.end(); ++cit) {
        Unit* candidate = cit->second;
        if (candidate == nullptr || !candidate->isAlive() || candidate->owner() == attacker.owner()) {
            continue;
        }
        const Position candidatePos = board_.findUnitOnBoard(candidate->id());
        if (!board_.inBounds(candidatePos)) {
            continue;
        }
        const int d = distanceSquared(attackerPos, candidatePos);
        if (best == nullptr || d < bestDist) {
            best = candidate;
            bestDist = d;
            bestPos = candidatePos;
            continue;
        }
        // 同距离时优先攻击低血量目标；再按棋盘坐标确定性打破平局。
        if (d == bestDist) {
            if (candidate->hp() < best->hp() || (candidate->hp() == best->hp() && candidatePos.col < bestPos.col) ||
                (candidate->hp() == best->hp() && candidatePos.col == bestPos.col && candidatePos.row > bestPos.row)) {
                best = candidate;
                bestPos = candidatePos;
            }
        }
    }
    return best;
}

// 流程：计算双方行列差 ──> 使用距离平方与射程平方比较 ──> 返回是否可攻击
bool BattleEngine::inRange(const Unit& attacker, Position attackerPos, Position targetPos) const {
    if (!board_.inBounds(attackerPos) || !board_.inBounds(targetPos)) {
        return false;
    }
    const int rangeSq = attacker.attackRange() * attacker.attackRange();
    return distanceSquared(attackerPos, targetPos) <= rangeSq;
}

int BattleEngine::distanceSquared(Position a, Position b) const {
    const int dr = a.row - b.row;
    const int dc = a.col - b.col;
    return dr * dr + dc * dc;
}

// 流程：当前位置 ──寻路──> 找到“下一步该去哪” ──> 清空旧格子 ──> 放到新格子
//                 （每 tick 只走 1 格；失败则回滚）
void BattleEngine::tryMoveTowardTarget(Unit* attacker, Unit* target) {
    const Position start = board_.findUnitOnBoard(attacker->id());
    const Position targetPos = board_.findUnitOnBoard(target->id());
    if (!board_.inBounds(start) || !board_.inBounds(targetPos)) {
        return;
    }
    Position next;
    if (!Pathfinder::nextStepTowardAttackRange(board_, units_, attacker->id(), start, targetPos, attacker->attackRange(),
                                               &next)) {
        return;
    }
    if (board_.occupantOnBoard(next) != Board::kEmptySlot) {
        return;
    }
    board_.clearOnBoard(start);
    if (!board_.placeOnBoard(attacker->id(), next)) {
        (void)board_.placeOnBoard(attacker->id(), start);
    }
}

// 流程：遍历 units_ ──> 发现死亡单位 ──> 标记 kDead ──> 清空棋盘占位 ──> 从 map 移除（不 delete）
void BattleEngine::clearDeadUnits() {
    for (std::map<int, Unit*>::iterator it = units_.begin(); it != units_.end();) {
        Unit* unit = it->second;
        if (unit == nullptr || unit->isAlive()) {
            ++it;
            continue;
        }
        unit->setState(UnitState::kDead);
        const Position pos = board_.findUnitOnBoard(unit->id());
        if (board_.inBounds(pos)) {
            board_.clearOnBoard(pos);
        }
        // BattleEngine 不持有指针所有权；内存释放由调用方（PvERoundRunner）负责。
        it = units_.erase(it);
    }
}

// 流程：遍历该阵营存活单位 ──> 若棋盘上仍有部署 ──> 返回 false（未全灭）
//       ──> 全部不在棋盘或已死 ──> 返回 true
bool BattleEngine::allDead(UnitOwner owner) const {
    // 只统计实际部署在棋盘格子上的单位（备战区上的单位不参与战斗，不计入死亡判断）。
    for (std::map<int, Unit*>::const_iterator it = units_.begin(); it != units_.end(); ++it) {
        Unit* unit = it->second;
        if (unit == nullptr || unit->owner() != owner || !unit->isAlive()) {
            continue;
        }
        const Position pos = board_.findUnitOnBoard(unit->id());
        if (board_.inBounds(pos)) {
            return false;  // 棋盘上仍有存活单位，未全灭
        }
    }
    return true;
}

// 流程：检查玩家/敌方是否全灭 ──> 双方仍存活则继续 ──> 标记 finished_
//       ──> 敌方全灭：玩家胜 ──> 否则：玩家负并设置扣血/金币
void BattleEngine::resolveEndState() {
    const bool playerDead = allDead(UnitOwner::player);
    const bool enemyDead = allDead(UnitOwner::enemy);
    if (!playerDead && !enemyDead) {
        return;
    }
    finished_ = true;
    if (enemyDead && !playerDead) {
        outcome_.playerWon = true;
        outcome_.goldReward = 3;
        outcome_.hpPenalty = 0;
        outcome_.gameOver = false;
    } else {
        outcome_.playerWon = false;
        outcome_.goldReward = 1;
        outcome_.hpPenalty = defeatHpPenalty_;
        outcome_.gameOver = false;  // 由 PvERoundRunner 在扣血后判断并设置
    }
}

}  // namespace core
}  // namespace my_auto_arena
