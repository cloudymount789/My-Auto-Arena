#include "ui/qt/ArenaScene.h"

#include <cmath>

#include <QBrush>
#include <QColor>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QPen>
#include <QPropertyAnimation>
#include <QTransform>

#include "ui/qt/TileGraphicsItem.h"
#include "ui/qt/UnitGraphicsItem.h"

namespace my_auto_arena {
namespace ui {

// 流程：设置场景尺寸 ──> 绘制棋盘格子 ──> 绘制备战区格子 ──> 同步已有单位图元 ──> 启动 VFX 定时器
//       （初始化整场 QGraphicsScene 布局与交互基础）
ArenaScene::ArenaScene(core::Board& board, core::Player& player, std::map<int, core::Unit*>& unitsMap,
                       QObject* parent)
    : QGraphicsScene(parent),
      board_(board),
      player_(player),
      unitsMap_(unitsMap),
      dragHandler_(board, player),
      mapper_(board.rows(), board.cols(), board.benchSize(), 64.0, 20.0, 20.0, 24.0),
      highlightedTile_(nullptr),
      dragEnabled_(true),
      vfxTimer_(nullptr) {
    setSceneRect(0.0, 0.0, 600.0, 700.0);

    for (int row = 0; row < board.rows(); ++row) {
        for (int col = 0; col < board.cols(); ++col) {
            const QRectF rect(20.0 + col * 64.0, 20.0 + row * 64.0, 64.0, 64.0);
            TileGraphicsItem::TileRegion region =
                board.isPlayerHalf(core::Position{row, col}) ? TileGraphicsItem::TileRegion::kBoardPlayer
                                                              : TileGraphicsItem::TileRegion::kBoardEnemy;
            TileGraphicsItem* tile = new TileGraphicsItem(region, row, col, rect);
            addItem(tile);
            tileItems_.push_back(tile);
        }
    }

    const double benchY = 20.0 + board.rows() * 64.0 + 24.0;
    for (int i = 0; i < board.benchSize(); ++i) {
        const QRectF rect(20.0 + i * 64.0, benchY, 64.0, 64.0);
        TileGraphicsItem* tile = new TileGraphicsItem(TileGraphicsItem::TileRegion::kBench, -1, i, rect);
        addItem(tile);
        tileItems_.push_back(tile);
    }

    for (std::map<int, core::Unit*>::iterator it = unitsMap_.begin(); it != unitsMap_.end(); ++it) {
        createUnitItem(it->second);
    }

    syncUnitPositions();

    vfxTimer_ = new QTimer(this);
    vfxTimer_->setSingleShot(false);
    vfxTimer_->setInterval(30);
    connect(vfxTimer_, SIGNAL(timeout()), this, SLOT(onVfxTick()));
}

UnitGraphicsItem* ArenaScene::createUnitItem(core::Unit* unit) {
    const bool isEnemy = (unit->owner() == core::UnitOwner::enemy);
    UnitGraphicsItem* item =
        new UnitGraphicsItem(unit->id(), QString::fromStdString(unit->name()), unit->hp(), unit->maxHp(),
                             unit->mana(), unit->maxMana(), 64.0, isEnemy, unit->starLevel());
    item->setZValue(10.0);
    connect(item, SIGNAL(dragMoved(int, QPointF)), this, SLOT(onDragMoved(int, QPointF)));
    connect(item, SIGNAL(dragFinished(int, QPointF)), this, SLOT(onDragFinished(int, QPointF)));
    connect(item, SIGNAL(unitClicked(int)), this, SLOT(onUnitClicked(int)));
    addItem(item);
    unitItems_[unit->id()] = item;
    return item;
}

void ArenaScene::addUnitItem(core::Unit* unit) {
    if (unitItems_.find(unit->id()) != unitItems_.end()) {
        return;
    }
    UnitGraphicsItem* item = createUnitItem(unit);
    double cx = 0.0;
    double cy = 0.0;
    const core::DragLocation loc = locateUnit(unit->id());
    mapper_.locationToPixelCenter(loc, cx, cy);
    const QRectF br = item->boundingRect();
    item->setPos(cx - br.width() / 2.0, cy - br.height() / 2.0);
    item->show();
}

// 流程：停止 VFX 定时器 ──> 删除静态特效图元 ──> 删除飞行物 ──> 删除脉冲环
//       （战斗 tick 切换或战后同步前清空所有瞬时视觉元素）
void ArenaScene::clearVfxItems() {
    if (vfxTimer_ != nullptr && vfxTimer_->isActive()) {
        vfxTimer_->stop();
    }
    for (std::size_t i = 0; i < vfxItems_.size(); ++i) {
        removeItem(vfxItems_.at(i));
        delete vfxItems_.at(i);
    }
    vfxItems_.clear();
    for (std::size_t i = 0; i < activeProjectiles_.size(); ++i) {
        if (activeProjectiles_.at(i).item != nullptr) {
            removeItem(activeProjectiles_.at(i).item);
            delete activeProjectiles_.at(i).item;
        }
    }
    activeProjectiles_.clear();
    for (std::size_t i = 0; i < activePulses_.size(); ++i) {
        if (activePulses_.at(i).item != nullptr) {
            removeItem(activePulses_.at(i).item);
            delete activePulses_.at(i).item;
        }
    }
    activePulses_.clear();
}

QPointF ArenaScene::tilePixelCenter(int row, int col) const {
    // 布局与构造函数一致：左边距 20px，格子 64px。
    const double cx = 20.0 + col * 64.0 + 32.0;
    const double cy = 20.0 + row * 64.0 + 32.0;
    return QPointF(cx, cy);
}

// ── 视觉特效颜色规则 ──────────────────────────────────────────────────────────────
// 玩家普通攻击：暖色系 (弓手=金黄, 法师=蓝紫)
// 敌方普通攻击：冷色系 (弓手=橙红, 法师=毒绿)
// 玩家技能：鲜艳职业色
// 敌方技能：暗化/反色版本，外加深红轮廓感

static QColor attackColor(core::UnitClass cls, core::UnitOwner owner) {
    if (owner == core::UnitOwner::player) {
        if (cls == core::UnitClass::kArcher)  return QColor(255, 220, 40, 230);   // 玩家射手：金黄
        if (cls == core::UnitClass::kMage)    return QColor(100, 80, 255, 230);   // 玩家法师：蓝紫
        return QColor(210, 210, 210, 200);
    }
    // 敌方
    if (cls == core::UnitClass::kArcher)  return QColor(255, 80, 20, 230);    // 敌方射手：橙红
    if (cls == core::UnitClass::kMage)    return QColor(30, 200, 80, 230);    // 敌方法师：毒绿
    return QColor(200, 60, 60, 200);    // 其他敌方：暗红
}

static QColor skillColor(core::UnitClass cls, core::UnitOwner owner) {
    if (owner == core::UnitOwner::player) {
        switch (cls) {
            case core::UnitClass::kWarrior: return QColor(255, 80, 20, 240);   // 血红
            case core::UnitClass::kArcher:  return QColor(255, 240, 0, 250);   // 亮金
            case core::UnitClass::kTank:    return QColor(255, 140, 0, 240);   // 橙
            case core::UnitClass::kMage:    return QColor(160, 0, 255, 245);   // 深紫
            case core::UnitClass::kHealer:  return QColor(0, 220, 120, 240);   // 翠绿
            default:                        return QColor(180, 180, 255, 220);
        }
    }
    // 敌方技能：暗化版
    switch (cls) {
        case core::UnitClass::kWarrior: return QColor(180, 0, 0, 240);      // 暗红
        case core::UnitClass::kArcher:  return QColor(200, 100, 0, 240);    // 暗橙
        case core::UnitClass::kTank:    return QColor(160, 60, 0, 240);     // 深棕橙
        case core::UnitClass::kMage:    return QColor(0, 160, 80, 245);     // 暗绿
        case core::UnitClass::kHealer:  return QColor(150, 0, 150, 240);    // 紫红
        default:                        return QColor(100, 100, 180, 220);
    }
}

// 生成一个飞行物并添加到场景。
// 宽度缩放 widthScale>1 表示沿飞行方向拉伸（箭矢效果），angle 为飞行方向角度（度）。
static QGraphicsEllipseItem* makeProjectileItem(QColor color, double r, double widthScale, double angle,
                                                 QGraphicsScene* scene) {
    // 宽度按比例拉伸，原点在中心。
    QGraphicsEllipseItem* item = new QGraphicsEllipseItem(-r * widthScale, -r, r * 2 * widthScale, r * 2);
    item->setBrush(QBrush(color));
    item->setPen(QPen(color.lighter(130), 1));
    item->setTransformOriginPoint(0.0, 0.0);
    item->setRotation(angle);
    item->setZValue(25.0);
    scene->addItem(item);
    return item;
}

// 流程：创建椭圆环图元 ──> 加入场景 ──> 填充脉冲参数并加入 activePulses_
//       （供技能施法/命中时生成扩散动画）
void ArenaScene::spawnPulse(QPointF center, QColor color, double startR, double endR, int duration) {
    QGraphicsEllipseItem* item = new QGraphicsEllipseItem(center.x() - startR, center.y() - startR,
                                                          startR * 2, startR * 2);
    item->setBrush(Qt::NoBrush);
    item->setPen(QPen(color, 2.5));
    item->setZValue(23.0);
    addItem(item);

    VfxPulse pulse;
    pulse.item        = item;
    pulse.center      = center;
    pulse.elapsed     = 0;
    pulse.duration    = duration;
    pulse.startRadius = startR;
    pulse.endRadius   = endR;
    pulse.color       = color;
    pulse.startAlpha  = color.alpha();
    activePulses_.push_back(pulse);
}

// 流程：遍历战斗事件 ──> 校验坐标并计算起止像素 ──> 按攻击/技能类型分支生成特效 ──> 按需启动 VFX 定时器
//       （将近战环、远程弹体、技能脉冲等映射到场景图元）
void ArenaScene::spawnVfx(const std::vector<core::BattleEvent>& events) {
    for (std::size_t i = 0; i < events.size(); ++i) {
        const core::BattleEvent& ev = events.at(i);

        const bool srcValid = (ev.srcRow >= 0 && ev.srcCol >= 0);
        const bool tgtValid = (ev.tgtRow >= 0 && ev.tgtCol >= 0);
        if (!srcValid || !tgtValid) {
            continue;
        }

        const QPointF src = tilePixelCenter(ev.srcRow, ev.srcCol);
        const QPointF tgt = tilePixelCenter(ev.tgtRow, ev.tgtCol);
        const double dx  = tgt.x() - src.x();
        const double dy  = tgt.y() - src.y();
        const double ang = std::atan2(dy, dx) * 180.0 / 3.14159265;

        const bool isEnemy = (ev.sourceOwner == core::UnitOwner::enemy);

        if (ev.type == core::BattleEvent::Type::kAttack) {
            if (ev.isMelee) {
                // ── 近战攻击 ──
                // 玩家：橙色冲击环 + 红点；敌方：暗红冲击环 + 深红点
                const QColor ringCol = isEnemy ? QColor(180, 20, 20, 200) : QColor(255, 120, 30, 200);
                const QColor dotCol  = isEnemy ? QColor(140, 0, 0, 200)   : QColor(255, 50, 50, 190);
                const double r = 24.0;
                QGraphicsEllipseItem* ring =
                    new QGraphicsEllipseItem(tgt.x() - r, tgt.y() - r, r * 2, r * 2);
                ring->setBrush(Qt::NoBrush);
                ring->setPen(QPen(ringCol, 3));
                ring->setZValue(20.0);
                addItem(ring);
                vfxItems_.push_back(ring);

                const double dotR = 9.0;
                QGraphicsEllipseItem* dot =
                    new QGraphicsEllipseItem(tgt.x() - dotR, tgt.y() - dotR, dotR * 2, dotR * 2);
                dot->setBrush(QBrush(dotCol));
                dot->setPen(Qt::NoPen);
                dot->setZValue(21.0);
                addItem(dot);
                vfxItems_.push_back(dot);
            } else {
                // ── 远程攻击：按职业和阵营选色，射手拉伸，法师圆形 ──
                const bool isArcher = (ev.sourceClass == core::UnitClass::kArcher);
                const QColor col    = attackColor(ev.sourceClass, ev.sourceOwner);
                const double r      = isArcher ? 5.0 : 9.0;
                const double wScale = isArcher ? 2.8 : 1.0;
                const int    dur    = isArcher ? 160 : 220;

                QGraphicsEllipseItem* bullet = makeProjectileItem(col, r, wScale, ang, this);
                bullet->setPos(src);

                VfxProjectile proj;
                proj.item            = bullet;
                proj.startPos        = src;
                proj.endPos          = tgt;
                proj.elapsed         = 0;
                proj.duration        = dur;
                proj.radius          = r;
                proj.widthScale      = wScale;
                proj.angle           = ang;
                proj.spawnImpactPulse = false;
                proj.impactColor     = col;
                activeProjectiles_.push_back(proj);
            }

        } else if (ev.type == core::BattleEvent::Type::kSkill) {
            // ── 技能：按 skillVfxType 分支渲染不同形态特效 ──
            const QColor skillCol = skillColor(ev.sourceClass, ev.sourceOwner);

            // 施法起点：发出 2 层扩散脉冲环
            spawnPulse(src, skillCol, 8.0,  44.0, 300);
            spawnPulse(src, skillCol, 14.0, 52.0, 420);

            if (ev.skillVfxType == core::BattleEvent::SkillVfxType::kStunSingle) {
                // 战士：单体眩晕 — 目标处金色星环 + 冲击点
                const QColor stunCol = isEnemy ? QColor(200, 180, 0, 230) : QColor(255, 230, 60, 240);
                spawnPulse(tgt, stunCol, 6.0,  38.0, 400);
                spawnPulse(tgt, stunCol, 12.0, 48.0, 520);
                const double dotR = 10.0;
                QGraphicsEllipseItem* dot =
                    new QGraphicsEllipseItem(tgt.x() - dotR, tgt.y() - dotR, dotR * 2, dotR * 2);
                dot->setBrush(QBrush(stunCol));
                dot->setPen(QPen(stunCol.lighter(150), 2));
                dot->setZValue(21.0);
                addItem(dot);
                vfxItems_.push_back(dot);
                continue;
            }

            if (ev.skillVfxType == core::BattleEvent::SkillVfxType::kAdjacentAoe) {
                // 重甲战士：范围伤害冲击圈
                const QColor aoeCol = isEnemy ? QColor(140, 60, 0, 180) : QColor(255, 140, 0, 180);
                spawnPulse(src, aoeCol, 20.0, 80.0, 500);
                spawnPulse(src, aoeCol, 30.0, 90.0, 600);
                continue;
            }

            if (ev.skillVfxType == core::BattleEvent::SkillVfxType::kHeal) {
                // 治疗师：绿色治疗波
                const QColor healCol = isEnemy ? QColor(160, 0, 160, 220) : QColor(0, 220, 120, 220);
                spawnPulse(tgt, healCol, 6.0,  36.0, 350);
                spawnPulse(tgt, healCol, 12.0, 44.0, 500);
                continue;
            }

            if (ev.skillVfxType == core::BattleEvent::SkillVfxType::kLineAoe) {
                // 射手：直线 AOE — 沿同行/同列绘制光束
                const QColor beamCol = isEnemy ? QColor(255, 120, 0, 210) : QColor(255, 240, 60, 230);
                const double tile = 64.0;
                const double margin = 20.0;
                const int row = ev.srcRow;
                const int col = ev.srcCol;
                QGraphicsLineItem* beam = nullptr;
                if (ev.lineIsVertical) {
                    const double x = margin + col * tile + tile / 2.0;
                    beam = new QGraphicsLineItem(x, margin, x, margin + 8 * tile);
                } else {
                    const double y = margin + row * tile + tile / 2.0;
                    beam = new QGraphicsLineItem(margin, y, margin + 8 * tile, y);
                }
                beam->setPen(QPen(beamCol, 6.0));
                beam->setZValue(22.0);
                addItem(beam);
                vfxItems_.push_back(beam);
                spawnPulse(src, beamCol, 10.0, 56.0, 380);
                continue;
            }

            if (ev.skillVfxType == core::BattleEvent::SkillVfxType::kRangeAoe) {
                // 法师：攻击范围 AOE — 大范围法术脉冲
                const QColor aoeCol = isEnemy ? QColor(0, 140, 70, 200) : QColor(160, 0, 255, 220);
                const double rangePx = 3.0 * 64.0;
                spawnPulse(src, aoeCol, 16.0, rangePx, 520);
                spawnPulse(src, aoeCol.lighter(130), 24.0, rangePx * 1.1, 620);
                continue;
            }

            // 兜底：默认技能弹体
            const double skillR = (ev.sourceClass == core::UnitClass::kMage) ? 13.0 : 8.0;
            const double sklWS  = (ev.sourceClass == core::UnitClass::kArcher) ? 2.5 : 1.0;
            const int    sklDur = (ev.sourceClass == core::UnitClass::kMage) ? 280 : 210;

            QGraphicsEllipseItem* sBullet = makeProjectileItem(skillCol, skillR, sklWS, ang, this);
            sBullet->setPos(src);

            VfxProjectile sproj;
            sproj.item            = sBullet;
            sproj.startPos        = src;
            sproj.endPos          = tgt;
            sproj.elapsed         = 0;
            sproj.duration        = sklDur;
            sproj.radius          = skillR;
            sproj.widthScale      = sklWS;
            sproj.angle           = ang;
            sproj.spawnImpactPulse = true;
            sproj.impactColor     = skillCol;
            activeProjectiles_.push_back(sproj);
        }
    }

    const bool needTimer = !activeProjectiles_.empty() || !activePulses_.empty();
    if (needTimer && vfxTimer_ != nullptr && !vfxTimer_->isActive()) {
        vfxTimer_->start();
    }
}

// 流程：推进飞行物插值位移 ──> 命中时生成闪光与爆炸环 ──> 推进脉冲环半径与透明度 ──> 清理已完成项并决定是否停表
//       （每 30ms 驱动所有活跃 VFX 动画）
void ArenaScene::onVfxTick() {
    const int delta = (vfxTimer_ != nullptr) ? vfxTimer_->interval() : 30;
    bool anyAlive = false;

    // ── 推进飞行物 ──────────────────────────────────────────────────────────
    for (std::size_t i = 0; i < activeProjectiles_.size(); ++i) {
        VfxProjectile& proj = activeProjectiles_.at(i);
        if (proj.item == nullptr) {
            continue;
        }
        proj.elapsed += delta;
        const double t = (proj.elapsed >= proj.duration)
                             ? 1.0
                             : static_cast<double>(proj.elapsed) / proj.duration;

        const QPointF cur = proj.startPos * (1.0 - t) + proj.endPos * t;
        proj.item->setPos(cur);

        if (proj.elapsed >= proj.duration) {
            removeItem(proj.item);
            delete proj.item;
            proj.item = nullptr;

            // 命中闪光（白色实心圆）
            const double hitR = proj.radius * 1.8;
            QGraphicsEllipseItem* hit =
                new QGraphicsEllipseItem(proj.endPos.x() - hitR, proj.endPos.y() - hitR, hitR * 2, hitR * 2);
            hit->setBrush(QBrush(QColor(255, 255, 200, 180)));
            hit->setPen(Qt::NoPen);
            hit->setZValue(22.0);
            addItem(hit);
            vfxItems_.push_back(hit);

            // 技能命中：额外产生两圈爆炸扩散环
            if (proj.spawnImpactPulse) {
                spawnPulse(proj.endPos, proj.impactColor, proj.radius, proj.radius * 4.5, 400);
                spawnPulse(proj.endPos, proj.impactColor.lighter(140), proj.radius * 0.5, proj.radius * 3.0, 280);
                anyAlive = true;  // 脉冲需要继续驱动定时器
            }
        } else {
            anyAlive = true;
        }
    }

    // 清理已完成的飞行物
    std::vector<VfxProjectile> aliveProj;
    for (std::size_t i = 0; i < activeProjectiles_.size(); ++i) {
        if (activeProjectiles_.at(i).item != nullptr) {
            aliveProj.push_back(activeProjectiles_.at(i));
        }
    }
    activeProjectiles_ = aliveProj;

    // ── 推进脉冲环 ──────────────────────────────────────────────────────────
    for (std::size_t i = 0; i < activePulses_.size(); ++i) {
        VfxPulse& pulse = activePulses_.at(i);
        if (pulse.item == nullptr) {
            continue;
        }
        pulse.elapsed += delta;
        const double t = (pulse.elapsed >= pulse.duration)
                             ? 1.0
                             : static_cast<double>(pulse.elapsed) / pulse.duration;

        // 半径插值
        const double r = pulse.startRadius + (pulse.endRadius - pulse.startRadius) * t;
        pulse.item->setRect(pulse.center.x() - r, pulse.center.y() - r, r * 2, r * 2);

        // 透明度从 startAlpha 线性衰减到 0
        const int alpha = static_cast<int>(pulse.startAlpha * (1.0 - t));
        QColor col = pulse.color;
        col.setAlpha(alpha);
        pulse.item->setPen(QPen(col, 2.5));

        if (pulse.elapsed >= pulse.duration) {
            removeItem(pulse.item);
            delete pulse.item;
            pulse.item = nullptr;
        } else {
            anyAlive = true;
        }
    }

    // 清理已完成的脉冲
    std::vector<VfxPulse> alivePulse;
    for (std::size_t i = 0; i < activePulses_.size(); ++i) {
        if (activePulses_.at(i).item != nullptr) {
            alivePulse.push_back(activePulses_.at(i));
        }
    }
    activePulses_ = alivePulse;

    if (!anyAlive && vfxTimer_ != nullptr) {
        vfxTimer_->stop();
    }
}

// 流程：清除上一 tick 特效 ──> 移除阵亡单位图元 ──> 刷新幸存单位血蓝与星级 ──> 同步位置
//       （战斗每 tick 后使 UI 与最新 unitsMap 对齐）
void ArenaScene::syncAfterBattle(const std::map<int, core::Unit*>& unitsMap) {
    clearVfxItems();  // 先清除上一 tick 的特效
    // 移除不再存在于 unitsMap 的图元（战斗中阵亡的单位）。
    std::vector<int> toRemove;
    for (std::map<int, UnitGraphicsItem*>::iterator it = unitItems_.begin(); it != unitItems_.end(); ++it) {
        if (unitsMap.find(it->first) == unitsMap.end()) {
            toRemove.push_back(it->first);
        }
    }
    for (std::size_t i = 0; i < toRemove.size(); ++i) {
        std::map<int, UnitGraphicsItem*>::iterator it = unitItems_.find(toRemove.at(i));
        if (it != unitItems_.end()) {
            removeItem(it->second);
            delete it->second;
            unitItems_.erase(it);
        }
    }
    // 刷新幸存单位的血蓝条、星级和位置。
    for (std::map<int, core::Unit*>::const_iterator it = unitsMap.begin(); it != unitsMap.end(); ++it) {
        const core::Unit* unit = it->second;
        std::map<int, UnitGraphicsItem*>::iterator itemIt = unitItems_.find(unit->id());
        if (itemIt != unitItems_.end()) {
            itemIt->second->setStats(unit->hp(), unit->maxHp(), unit->mana(), unit->maxMana());
            itemIt->second->setStarLevel(unit->starLevel());  // 升星后及时刷新★显示
        }
    }
    syncUnitPositions();
}

void ArenaScene::setDragEnabled(bool enabled) { dragEnabled_ = enabled; }

const core::Unit* ArenaScene::unitById(int unitId) const {
    std::map<int, core::Unit*>::const_iterator it = unitsMap_.find(unitId);
    if (it != unitsMap_.end()) {
        return it->second;
    }
    return nullptr;
}

void ArenaScene::rebuild() { syncUnitPositions(); }

// 流程：像素坐标转棋盘/备战区位置 ──> 遍历格子匹配目标 ──> 高亮或清除高亮
//       （拖拽移动过程中实时反馈可放置格）
void ArenaScene::onDragMoved(int, QPointF scenePos) {
    if (!dragEnabled_) {
        return;
    }
    core::DragLocation to = core::DragLocation::fromBench(0);
    if (!mapper_.pixelToLocation(scenePos.x(), scenePos.y(), to)) {
        clearTileHighlight();
        return;
    }

    for (std::size_t i = 0; i < tileItems_.size(); ++i) {
        TileGraphicsItem* tile = tileItems_.at(i);
        bool hit = false;
        if (to.type == core::DragLocation::kBoard) {
            hit = (tile->logicalRow() == to.boardPos.row && tile->logicalCol() == to.boardPos.col);
        } else {
            hit = (tile->logicalRow() == -1 && tile->logicalCol() == to.benchIndex);
        }

        if (hit) {
            if (highlightedTile_ != tile) {
                clearTileHighlight();
                highlightedTile_ = tile;
                highlightedTile_->setHighlighted(true);
            }
            return;
        }
    }

    clearTileHighlight();
}

// 流程：校验拖拽开关与单位 ──> 定位源位置 ──> 解析释放坐标 ──> 执行拖放逻辑 ──> 同步或回弹并上报结果
//       （拖拽结束时的完整放置/交换/失败处理）
void ArenaScene::onDragFinished(int unitId, QPointF releaseScenePos) {
    if (!dragEnabled_) {
        std::map<int, UnitGraphicsItem*>::iterator it = unitItems_.find(unitId);
        if (it != unitItems_.end()) {
            snapBack(it->second);
        }
        clearTileHighlight();
        return;
    }

    std::map<int, UnitGraphicsItem*>::iterator it = unitItems_.find(unitId);
    if (it == unitItems_.end()) {
        return;
    }
    UnitGraphicsItem* item = it->second;

    core::DragLocation from = locateUnit(unitId);
    if (from.type == core::DragLocation::kBench && from.benchIndex < 0) {
        snapBack(item);
        clearTileHighlight();
        emit dragResultReady(core::DragResult::kInvalidSource);
        return;
    }
    core::DragLocation to = core::DragLocation::fromBench(0);
    if (!mapper_.pixelToLocation(releaseScenePos.x(), releaseScenePos.y(), to)) {
        snapBack(item);
        clearTileHighlight();
        emit dragResultReady(core::DragResult::kOutOfBounds);
        return;
    }

    const core::DragResult result = dragHandler_.execute(from, to);
    if (result == core::DragResult::kSuccess || result == core::DragResult::kSwapped ||
        result == core::DragResult::kSameLocation) {
        syncUnitPositions();
    } else {
        snapBack(item);
    }
    clearTileHighlight();
    emit dragResultReady(result);
}

void ArenaScene::onUnitClicked(int unitId) { emit unitSelected(unitId); }

// 流程：扫描棋盘占用 ──> 未找到则扫描备战区 ──> 返回对应 DragLocation
//       （根据 unitId 反查单位当前所在区域）
core::DragLocation ArenaScene::locateUnit(int unitId) const {
    for (int row = 0; row < board_.rows(); ++row) {
        for (int col = 0; col < board_.cols(); ++col) {
            if (board_.occupantOnBoard(core::Position{row, col}) == unitId) {
                return core::DragLocation::fromBoard(row, col);
            }
        }
    }
    for (int i = 0; i < board_.benchSize(); ++i) {
        if (board_.occupantOnBench(i) == unitId) {
            return core::DragLocation::fromBench(i);
        }
    }
    return core::DragLocation::fromBench(-1);
}

// 流程：遍历所有单位图元 ──> 定位逻辑坐标 ──> 无效则隐藏，有效则居中摆放并显示
//       （拖放、购买、战后等场景变化后统一刷新像素位置）
void ArenaScene::syncUnitPositions() {
    for (std::map<int, UnitGraphicsItem*>::iterator it = unitItems_.begin(); it != unitItems_.end(); ++it) {
        const int unitId = it->first;
        UnitGraphicsItem* item = it->second;
        const core::DragLocation location = locateUnit(unitId);
        if ((location.type == core::DragLocation::kBench && location.benchIndex < 0) ||
            (location.type == core::DragLocation::kBoard && !board_.inBounds(location.boardPos))) {
            item->hide();
            continue;
        }

        double cx = 0.0;
        double cy = 0.0;
        mapper_.locationToPixelCenter(location, cx, cy);
        const QRectF br = item->boundingRect();
        item->setPos(cx - br.width() / 2.0, cy - br.height() / 2.0);
        item->show();
    }
}

void ArenaScene::snapBack(UnitGraphicsItem* item) {
    QPropertyAnimation* animation = new QPropertyAnimation(item, "pos", item);
    animation->setDuration(200);
    animation->setStartValue(item->pos());
    animation->setEndValue(item->dragStartScenePos());
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void ArenaScene::clearTileHighlight() {
    if (highlightedTile_ != nullptr) {
        highlightedTile_->setHighlighted(false);
        highlightedTile_ = nullptr;
    }
}

}  // namespace ui
}  // namespace my_auto_arena
