#include "core/Unit.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "core/Board.h"

namespace my_auto_arena {
namespace core {

namespace {

const int kBaseAttackSpeed = 100;
const int kShieldTriggerDamageTicks = 4;
const int kShieldImmuneTicks = 1;

int ownerIndex(UnitOwner owner) {
    return owner == UnitOwner::player ? 0 : 1;
}

}  // namespace

int gCurrentSynergyTick = 0;
int gShieldDamageTicks[2] = {0, 0};
int gShieldLastDamageTick[2] = {-1, -1};
int gShieldImmuneTicks[2] = {0, 0};
int gShieldActivatedTick[2] = {-1, -1};

// 流程：接收基础属性 ──> 初始化基础/当前属性缓存 ──> 校验参数 ──> 统一重算最终属性并回满血
Unit::Unit(int id, std::string name, UnitOwner owner, int maxHp, int attack, int attackRange, int maxMana,
           UnitClass unitClass)
    : id_(id),
      name_(std::move(name)),
      owner_(owner),
      hp_(maxHp),
      baseMaxHp_(maxHp),
      baseAttack_(attack),
      attackRange_(attackRange),
      mana_(0),
      maxMana_(maxMana),
      unitClass_(unitClass),
      starLevel_(1),
      equippedItems_(),
      bonusAtk_(0),
      bonusMagAtk_(0),
      bonusMaxHp_(0),
      bonusPhysicalDef_(0),
      bonusMagicDef_(0),
      armorBreak_(false),
      magicPenetration_(false),
      shieldField_(false),
      star1Atk_(attack),
      star1MaxHp_(maxHp),
      star1MagAtk_(0),
      baseMagicAtk_(0),
      basePhysicalDef_(0),
      baseMagicDef_(0),
      star1PhysDef_(0),
      star1MagDef_(0),
      state_(UnitState::kIdle),
      stunTicksRemaining_(0),
      currentPhysicalAtk_(attack),
      currentMagicAtk_(0),
      currentMaxHp_(maxHp),
      currentPhysicalDef_(0),
      currentMagicDef_(0),
      currentMaxMana_(maxMana),
      currentAttackSpeed_(kBaseAttackSpeed) {
    if (id < 0 || maxHp <= 0 || attack < 0 || attackRange <= 0 || maxMana <= 0) {
        throw std::invalid_argument("Invalid unit stats.");
    }
    recalculateCurrentStats();
    hp_ = currentMaxHp_;
}

// 流程：逐字段复制基础属性/装备/羁绊/当前缓存 ──> 得到与来源单位状态一致的新对象
Unit::Unit(const Unit& other)
    : id_(other.id_),
      name_(other.name_),
      owner_(other.owner_),
      hp_(other.hp_),
      baseMaxHp_(other.baseMaxHp_),
      baseAttack_(other.baseAttack_),
      attackRange_(other.attackRange_),
      mana_(other.mana_),
      maxMana_(other.maxMana_),
      unitClass_(other.unitClass_),
      starLevel_(other.starLevel_),
      equippedItems_(other.equippedItems_),
      bonusAtk_(other.bonusAtk_),
      bonusMagAtk_(other.bonusMagAtk_),
      bonusMaxHp_(other.bonusMaxHp_),
      bonusPhysicalDef_(other.bonusPhysicalDef_),
      bonusMagicDef_(other.bonusMagicDef_),
      armorBreak_(other.armorBreak_),
      magicPenetration_(other.magicPenetration_),
      shieldField_(other.shieldField_),
      star1Atk_(other.star1Atk_),
      star1MaxHp_(other.star1MaxHp_),
      star1MagAtk_(other.star1MagAtk_),
      baseMagicAtk_(other.baseMagicAtk_),
      basePhysicalDef_(other.basePhysicalDef_),
      baseMagicDef_(other.baseMagicDef_),
      star1PhysDef_(other.star1PhysDef_),
      star1MagDef_(other.star1MagDef_),
      state_(other.state_),
      stunTicksRemaining_(other.stunTicksRemaining_),
      currentPhysicalAtk_(other.currentPhysicalAtk_),
      currentMagicAtk_(other.currentMagicAtk_),
      currentMaxHp_(other.currentMaxHp_),
      currentPhysicalDef_(other.currentPhysicalDef_),
      currentMagicDef_(other.currentMagicDef_),
      currentMaxMana_(other.currentMaxMana_),
      currentAttackSpeed_(other.currentAttackSpeed_) {}

// 流程：自赋值保护 ──> 逐字段覆盖本对象状态 ──> 返回当前对象引用
Unit& Unit::operator=(const Unit& other) {
    if (this == &other) {
        return *this;
    }
    id_ = other.id_;
    name_ = other.name_;
    owner_ = other.owner_;
    hp_ = other.hp_;
    baseMaxHp_ = other.baseMaxHp_;
    baseAttack_ = other.baseAttack_;
    attackRange_ = other.attackRange_;
    mana_ = other.mana_;
    maxMana_ = other.maxMana_;
    unitClass_ = other.unitClass_;
    starLevel_ = other.starLevel_;
    equippedItems_ = other.equippedItems_;
    bonusAtk_ = other.bonusAtk_;
    bonusMagAtk_ = other.bonusMagAtk_;
    bonusMaxHp_ = other.bonusMaxHp_;
    bonusPhysicalDef_ = other.bonusPhysicalDef_;
    bonusMagicDef_ = other.bonusMagicDef_;
    armorBreak_ = other.armorBreak_;
    magicPenetration_ = other.magicPenetration_;
    shieldField_ = other.shieldField_;
    star1Atk_ = other.star1Atk_;
    star1MaxHp_ = other.star1MaxHp_;
    star1MagAtk_ = other.star1MagAtk_;
    baseMagicAtk_ = other.baseMagicAtk_;
    basePhysicalDef_ = other.basePhysicalDef_;
    baseMagicDef_ = other.baseMagicDef_;
    star1PhysDef_ = other.star1PhysDef_;
    star1MagDef_ = other.star1MagDef_;
    state_ = other.state_;
    stunTicksRemaining_ = other.stunTicksRemaining_;
    currentPhysicalAtk_ = other.currentPhysicalAtk_;
    currentMagicAtk_ = other.currentMagicAtk_;
    currentMaxHp_ = other.currentMaxHp_;
    currentPhysicalDef_ = other.currentPhysicalDef_;
    currentMagicDef_ = other.currentMagicDef_;
    currentMaxMana_ = other.currentMaxMana_;
    currentAttackSpeed_ = other.currentAttackSpeed_;
    return *this;
}

int Unit::roundStat(double value) {
    return static_cast<int>(value + (value >= 0.0 ? 0.5 : -0.5));
}

int Unit::id() const { return id_; }
const std::string& Unit::name() const { return name_; }
UnitOwner Unit::owner() const { return owner_; }
int Unit::hp() const { return hp_; }
int Unit::maxHp() const { return currentMaxHp_; }
int Unit::attack() const { return physicalAtk(); }
int Unit::physicalAtk() const { return currentPhysicalAtk_; }
int Unit::magicAtk() const { return currentMagicAtk_; }
int Unit::physicalDef() const { return currentPhysicalDef_; }
int Unit::magicDef() const { return currentMagicDef_; }
int Unit::attackSpeed() const { return currentAttackSpeed_; }
int Unit::attackRange() const { return attackRange_; }
int Unit::mana() const { return mana_; }
int Unit::maxMana() const { return currentMaxMana_; }
bool Unit::isAlive() const { return hp_ > 0; }

bool Unit::isStunned() const { return stunTicksRemaining_ > 0; }

int Unit::stunTicksRemaining() const { return stunTicksRemaining_; }

// 流程：过滤无效眩晕与死亡单位 ──> 仅在新眩晕更长时覆盖剩余 tick
void Unit::applyStun(int ticks) {
    if (ticks <= 0 || !isAlive()) {
        return;
    }
    if (ticks > stunTicksRemaining_) {
        stunTicksRemaining_ = ticks;
    }
}

void Unit::tickStun() {
    if (stunTicksRemaining_ > 0) {
        --stunTicksRemaining_;
    }
}

int Unit::basePhysicalAtk() const { return baseAttack_; }
int Unit::baseMagicAtk() const { return baseMagicAtk_; }
int Unit::baseMaxHp() const { return baseMaxHp_; }
int Unit::basePhysicalDef() const { return basePhysicalDef_; }
int Unit::baseMagicDef() const { return baseMagicDef_; }
int Unit::baseMaxMana() const { return maxMana_; }
int Unit::baseAttackSpeed() const { return kBaseAttackSpeed; }

UnitClass Unit::unitClass() const { return unitClass_; }
int Unit::starLevel() const { return starLevel_; }
const std::vector<ItemType>& Unit::equippedItems() const { return equippedItems_; }
// 流程：兼容旧接口 ──> 无装备返回 kNone ──> 有装备返回第一个槽位
ItemType Unit::equippedItem() const {
    if (equippedItems_.empty()) {
        return ItemType::kNone;
    }
    return equippedItems_.at(0);
}
int Unit::equipSlotCount() const { return starLevel_; }

UnitState Unit::state() const { return state_; }
void Unit::setState(UnitState s) { state_ = s; }

// 流程：过滤无效伤害/死亡单位 ──> 检查护盾免伤 ──> 扣血 ──> 按阵营累计护盾触发计数
void Unit::takeDamage(int amount) {
    if (amount <= 0 || !isAlive()) {
        return;
    }
    const int idx = ownerIndex(owner_);
    if (gShieldImmuneTicks[idx] > 0) {
        return;
    }
    hp_ = std::max(0, hp_ - amount);
    if (shieldField_) {
        if (gShieldLastDamageTick[idx] != gCurrentSynergyTick) {
            gShieldLastDamageTick[idx] = gCurrentSynergyTick;
            ++gShieldDamageTicks[idx];
            if (gShieldDamageTicks[idx] >= kShieldTriggerDamageTicks) {
                gShieldDamageTicks[idx] = 0;
                gShieldImmuneTicks[idx] = kShieldImmuneTicks;
                gShieldActivatedTick[idx] = gCurrentSynergyTick;
            }
        }
    }
}

void Unit::takePhysicalDamage(int rawDmg) {
    takePhysicalDamage(rawDmg, 0);
}

// 流程：限制防御忽略百分比 ──> 计算有效物防与净伤害 ──> 交给通用 takeDamage 扣血
void Unit::takePhysicalDamage(int rawDmg, int defenseIgnorePercent) {
    const int ignore = std::max(0, std::min(100, defenseIgnorePercent));
    const int effectiveDef = roundStat(physicalDef() * (100 - ignore) / 100.0);
    const int net = std::max(1, rawDmg - effectiveDef);
    takeDamage(net);
}

void Unit::takeMagicDamage(int rawDmg) {
    takeMagicDamage(rawDmg, 0);
}

// 流程：限制防御忽略百分比 ──> 计算有效魔防与净伤害 ──> 交给通用 takeDamage 扣血
void Unit::takeMagicDamage(int rawDmg, int defenseIgnorePercent) {
    const int ignore = std::max(0, std::min(100, defenseIgnorePercent));
    const int effectiveDef = roundStat(magicDef() * (100 - ignore) / 100.0);
    const int net = std::max(1, rawDmg - effectiveDef);
    takeDamage(net);
}

// 流程：过滤无效回蓝与死亡单位 ──> 增加法力 ──> 不超过当前最大法力
void Unit::gainMana(int amount) {
    if (amount <= 0 || !isAlive()) {
        return;
    }
    mana_ = std::min(maxMana(), mana_ + amount);
}

// 流程：过滤无效治疗与死亡单位 ──> 增加生命 ──> 不超过当前最大生命
void Unit::heal(int amount) {
    if (amount <= 0 || !isAlive()) {
        return;
    }
    hp_ = std::min(maxHp(), hp_ + amount);
}

void Unit::spendAllMana() { mana_ = 0; }

// 流程：重算装备/羁绊后的最终属性 ──> 生命回满 ──> 清空蓝量与眩晕状态
void Unit::resetToFull() {
    recalculateCurrentStats();
    hp_ = maxHp();
    mana_ = 0;
    stunTicksRemaining_ = 0;
}

// 流程：校验装备类型 ──> 有空槽则追加、满槽则替换首槽 ──> 重算属性 ──> 按最大生命变化等比修正当前血量
void Unit::equipItem(ItemType item) {
    if (item == ItemType::kNone) {
        return;
    }

    const int oldMax = maxHp();
    if (static_cast<int>(equippedItems_.size()) < equipSlotCount()) {
        equippedItems_.push_back(item);
    } else {
        equippedItems_.at(0) = item;
    }

    recalculateCurrentStats();

    const int newMax = maxHp();
    if (newMax > oldMax && oldMax > 0 && hp_ > 0) {
        hp_ = roundStat(static_cast<double>(hp_) / oldMax * newMax);
    }
    clampHpToCurrentMax();
    clampManaToCurrentMax();
}

void Unit::unequipItem() { unequipItemAt(0); }

// 流程：校验槽位 ──> 从装备列表移除 ──> 重算最终属性 ──> 限制当前血蓝不超过新上限
void Unit::unequipItemAt(int slotIndex) {
    if (slotIndex < 0 || slotIndex >= static_cast<int>(equippedItems_.size())) {
        return;
    }
    equippedItems_.erase(equippedItems_.begin() + slotIndex);
    recalculateCurrentStats();
    clampHpToCurrentMax();
    clampManaToCurrentMax();
}

// 流程：复制当前装备作为返回值 ──> 清空装备槽 ──> 重算属性并夹紧血蓝 ──> 返回卸下列表
std::vector<ItemType> Unit::takeAllEquippedItems() {
    std::vector<ItemType> items = equippedItems_;
    equippedItems_.clear();
    recalculateCurrentStats();
    clampHpToCurrentMax();
    clampManaToCurrentMax();
    return items;
}

// 流程：保存旧最大生命 ──> 写入各类羁绊加成/标记 ──> 重算属性 ──> 最大生命提升时等比抬高当前血量
void Unit::setSynergyBuffs(int bonusAtk, int bonusMagAtk, int bonusMaxHp,
                           int bonusPhysicalDef, int bonusMagicDef,
                           bool armorBreak, bool magicPenetration, bool shieldField) {
    const int oldMax = maxHp();
    bonusAtk_ = bonusAtk;
    bonusMagAtk_ = bonusMagAtk;
    bonusMaxHp_ = bonusMaxHp;
    bonusPhysicalDef_ = bonusPhysicalDef;
    bonusMagicDef_ = bonusMagicDef;
    armorBreak_ = armorBreak;
    magicPenetration_ = magicPenetration;
    shieldField_ = shieldField;
    recalculateCurrentStats();
    const int newMax = maxHp();
    if (newMax > oldMax && oldMax > 0 && hp_ > 0) {
        hp_ = roundStat(static_cast<double>(hp_) / oldMax * newMax);
    }
    clampHpToCurrentMax();
}

// 流程：清零所有羁绊数值与特殊标记 ──> 重算最终属性 ──> 夹紧当前生命
void Unit::clearSynergyBuffs() {
    bonusAtk_ = 0;
    bonusMagAtk_ = 0;
    bonusMaxHp_ = 0;
    bonusPhysicalDef_ = 0;
    bonusMagicDef_ = 0;
    armorBreak_ = false;
    magicPenetration_ = false;
    shieldField_ = false;
    recalculateCurrentStats();
    clampHpToCurrentMax();
}

bool Unit::hasArmorBreak() const { return armorBreak_; }
bool Unit::hasMagicPenetration() const { return magicPenetration_; }
bool Unit::hasShieldField() const { return shieldField_; }
int Unit::physicalDefenseIgnorePercent() const { return armorBreak_ ? 30 : 0; }
int Unit::magicDefenseIgnorePercent() const { return magicPenetration_ ? 100 : 0; }

void Unit::beginSynergyDamageTick(int tick) { gCurrentSynergyTick = tick; }

// 流程：遍历双方阵营免伤计时 ──> 只在触发 tick 之后递减 ──> 让护盾至少覆盖完整 1 tick
void Unit::endSynergyDamageTick() {
    for (int i = 0; i < 2; ++i) {
        if (gShieldImmuneTicks[i] > 0 && gShieldActivatedTick[i] < gCurrentSynergyTick) {
            --gShieldImmuneTicks[i];
        }
    }
}

// 流程：重置当前羁绊 tick ──> 清空双方受击计数/上次受击 tick/免伤剩余时间/触发 tick
void Unit::resetSynergyShieldState() {
    gCurrentSynergyTick = 0;
    for (int i = 0; i < 2; ++i) {
        gShieldDamageTicks[i] = 0;
        gShieldLastDamageTick[i] = -1;
        gShieldImmuneTicks[i] = 0;
        gShieldActivatedTick[i] = -1;
    }
}

// 流程：根据目标星级选倍率 ──> 缩放基础攻血与防御/法攻 ──> 更新星级 ──> 重算属性并回满血
void Unit::upgradeToStar(int newStarLevel) {
    double factor = (newStarLevel == 2) ? 3.0 : 7.0;
    baseAttack_ = static_cast<int>(star1Atk_ * factor);
    baseMaxHp_ = static_cast<int>(star1MaxHp_ * factor);
    if (star1MagAtk_ > 0) {
        baseMagicAtk_ = static_cast<int>(star1MagAtk_ * factor);
    }
    if (star1PhysDef_ > 0) {
        basePhysicalDef_ = static_cast<int>(star1PhysDef_ * factor);
    }
    if (star1MagDef_ > 0) {
        baseMagicDef_ = static_cast<int>(star1MagDef_ * factor);
    }
    starLevel_ = newStarLevel;
    recalculateCurrentStats();
    hp_ = maxHp();
}

int Unit::scaledSkillDamage(int baseDamage) const {
    if (starLevel_ == 2) return static_cast<int>(baseDamage * 3.0);
    if (starLevel_ == 3) return static_cast<int>(baseDamage * 7.0);
    return baseDamage;
}

// 流程：校验目标与双方坐标 ──> 判断是否在圆形射程内 ──> 按普攻类型走物理或法术伤害通道
void Unit::performAttackInRange(Board& board, Unit* primaryTarget) {
    if (primaryTarget == nullptr || !primaryTarget->isAlive()) {
        return;
    }
    const Position selfPos = board.findUnitOnBoard(id());
    const Position tgtPos = board.findUnitOnBoard(primaryTarget->id());
    if (!board.inBounds(selfPos) || !board.inBounds(tgtPos)) {
        return;
    }
    const int dr = selfPos.row - tgtPos.row;
    const int dc = selfPos.col - tgtPos.col;
    const int r = attackRange();
    if (dr * dr + dc * dc <= r * r) {
        if (usesMagicAttack()) {
            primaryTarget->takeMagicDamage(magicAtk(), magicDefenseIgnorePercent());
        } else {
            primaryTarget->takePhysicalDamage(physicalAtk(), physicalDefenseIgnorePercent());
        }
    }
}

// 流程：基类默认技能不产生效果 ──> 标记参数已使用 ──> 清空法力，供未重写子类保持安全行为
void Unit::castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) {
    (void)board;
    (void)units;
    (void)primaryTarget;
    spendAllMana();
}

// 流程：遍历装备槽 ──> 读取装备定义 ──> 按基础物攻百分比累加装备物攻加成
int Unit::equipmentBonusPhysAtk() const {
    int total = 0;
    for (std::size_t i = 0; i < equippedItems_.size(); ++i) {
        const ItemDef& def = getItemDef(equippedItems_.at(i));
        total += roundStat(baseAttack_ * def.bonusPhysAtkPercent / 100.0);
    }
    return total;
}

// 流程：遍历装备槽 ──> 读取装备定义 ──> 按基础法攻百分比累加装备法攻加成
int Unit::equipmentBonusMagAtk() const {
    int total = 0;
    for (std::size_t i = 0; i < equippedItems_.size(); ++i) {
        const ItemDef& def = getItemDef(equippedItems_.at(i));
        total += roundStat(baseMagicAtk_ * def.bonusMagAtkPercent / 100.0);
    }
    return total;
}

// 流程：遍历装备槽 ──> 读取装备定义 ──> 按基础物防百分比累加装备物防加成
int Unit::equipmentBonusPhysDef() const {
    int total = 0;
    for (std::size_t i = 0; i < equippedItems_.size(); ++i) {
        const ItemDef& def = getItemDef(equippedItems_.at(i));
        total += roundStat(basePhysicalDef_ * def.bonusPhysDefensePercent / 100.0);
    }
    return total;
}

// 流程：遍历装备槽 ──> 读取装备定义 ──> 按基础魔防百分比累加装备魔防加成
int Unit::equipmentBonusMagDef() const {
    int total = 0;
    for (std::size_t i = 0; i < equippedItems_.size(); ++i) {
        const ItemDef& def = getItemDef(equippedItems_.at(i));
        total += roundStat(baseMagicDef_ * def.bonusMagDefensePercent / 100.0);
    }
    return total;
}

// 流程：遍历装备槽 ──> 读取装备定义 ──> 按基础生命百分比累加装备生命加成
int Unit::equipmentBonusMaxHp() const {
    int total = 0;
    for (std::size_t i = 0; i < equippedItems_.size(); ++i) {
        const ItemDef& def = getItemDef(equippedItems_.at(i));
        total += roundStat(baseMaxHp_ * def.bonusMaxHpPercent / 100.0);
    }
    return total;
}

// 流程：遍历装备槽 ──> 读取装备定义 ──> 按基础攻速百分比累加装备攻速加成
int Unit::equipmentBonusAttackSpeed() const {
    int total = 0;
    for (std::size_t i = 0; i < equippedItems_.size(); ++i) {
        const ItemDef& def = getItemDef(equippedItems_.at(i));
        total += roundStat(kBaseAttackSpeed * def.bonusAttackSpeedPercent / 100.0);
    }
    return total;
}

// 流程：遍历装备槽 ──> 累加固定最大法力修正值 ──> 返回蓝量上限的装备修正
int Unit::equipmentBonusMaxManaFlat() const {
    int total = 0;
    for (std::size_t i = 0; i < equippedItems_.size(); ++i) {
        total += getItemDef(equippedItems_.at(i)).bonusMaxManaFlat;
    }
    return total;
}

// 流程：先计算所有装备加成 ──> 基础属性+装备+羁绊得到当前属性缓存 ──> 单独处理法力下限
void Unit::recalculateCurrentStats() {
    const int equipPhysAtk = equipmentBonusPhysAtk();
    const int equipMagAtk = equipmentBonusMagAtk();
    const int equipMaxHp = equipmentBonusMaxHp();
    const int equipPhysDef = equipmentBonusPhysDef();
    const int equipMagDef = equipmentBonusMagDef();
    const int equipAtkSpeed = equipmentBonusAttackSpeed();

    currentPhysicalAtk_ = roundStat(baseAttack_ + equipPhysAtk + bonusAtk_);
    currentMagicAtk_ = roundStat(baseMagicAtk_ + equipMagAtk + bonusMagAtk_);
    currentMaxHp_ = roundStat(baseMaxHp_ + equipMaxHp + bonusMaxHp_);
    currentPhysicalDef_ = roundStat(basePhysicalDef_ + equipPhysDef + bonusPhysicalDef_);
    currentMagicDef_ = roundStat(baseMagicDef_ + equipMagDef + bonusMagicDef_);
    currentAttackSpeed_ = roundStat(kBaseAttackSpeed + equipAtkSpeed);

    const int rawMaxMana = maxMana_ + equipmentBonusMaxManaFlat();
    currentMaxMana_ = rawMaxMana < 1 ? 1 : rawMaxMana;
}

int Unit::equipmentMagicAtkBonus() const { return equipmentBonusMagAtk(); }

void Unit::setBaseMagicAtk(int v) {
    baseMagicAtk_ = v;
    star1MagAtk_ = v;
    recalculateCurrentStats();
}
void Unit::setBasePhysicalDef(int v) {
    basePhysicalDef_ = v;
    star1PhysDef_ = v;
    recalculateCurrentStats();
}
void Unit::setBaseMagicDef(int v) {
    baseMagicDef_ = v;
    star1MagDef_ = v;
    recalculateCurrentStats();
}

// 流程：读取当前最大生命 ──> 当前血量高于上限则压回上限 ──> 低于0则归零
void Unit::clampHpToCurrentMax() {
    const int currentMaxHp = maxHp();
    if (hp_ > currentMaxHp) {
        hp_ = currentMaxHp;
    }
    if (hp_ < 0) {
        hp_ = 0;
    }
}

// 流程：当前蓝量高于最大法力则压回上限 ──> 低于0则归零
void Unit::clampManaToCurrentMax() {
    if (mana_ > maxMana()) {
        mana_ = maxMana();
    }
    if (mana_ < 0) {
        mana_ = 0;
    }
}

bool Unit::usesMagicAttack() const { return false; }

PhysicalAttackUnit::PhysicalAttackUnit(int id, const std::string& name, UnitOwner owner,
                                       int maxHp, int attack, int attackRange, int maxMana,
                                       UnitClass unitClass)
    : Unit(id, name, owner, maxHp, attack, attackRange, maxMana, unitClass) {}

PhysicalAttackUnit::PhysicalAttackUnit(const PhysicalAttackUnit& other) : Unit(other) {}

bool PhysicalAttackUnit::usesMagicAttack() const { return false; }

MagicalAttackUnit::MagicalAttackUnit(int id, const std::string& name, UnitOwner owner,
                                     int maxHp, int attack, int attackRange, int maxMana,
                                     UnitClass unitClass)
    : Unit(id, name, owner, maxHp, attack, attackRange, maxMana, unitClass) {
    setBaseMagicAtk(attack);
}

MagicalAttackUnit::MagicalAttackUnit(const MagicalAttackUnit& other) : Unit(other) {}

bool MagicalAttackUnit::usesMagicAttack() const { return true; }

WarriorUnit::WarriorUnit(int id, UnitOwner owner)
    : PhysicalAttackUnit(id, "Warrior", owner, 800, 65, 1, 100, UnitClass::kWarrior) {}

WarriorUnit::WarriorUnit(const WarriorUnit& other) : PhysicalAttackUnit(other) {}

void WarriorUnit::castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) {
    (void)units;
    performAttackInRange(board, primaryTarget);
    spendAllMana();
}

MageUnit::MageUnit(int id, UnitOwner owner)
    : MagicalAttackUnit(id, "Mage", owner, 500, 45, 3, 100, UnitClass::kMage) {}

MageUnit::MageUnit(const MageUnit& other) : MagicalAttackUnit(other) {}

void MageUnit::castFullManaSkill(Board& board, std::map<int, Unit*>& units, Unit* primaryTarget) {
    (void)units;
    performAttackInRange(board, primaryTarget);
    spendAllMana();
}

}  // namespace core
}  // namespace my_auto_arena
