#include "ui/qt/UnitGraphicsItem.h"

#include <QBrush>
#include <QCursor>
#include <QFont>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPen>

namespace my_auto_arena {
namespace ui {

UnitGraphicsItem::UnitGraphicsItem(int unitId, const QString& name, int hp, int maxHp, int mana, int maxMana,
                                   double tileSize)
    : unitId_(unitId),
      name_(name),
      hp_(hp),
      maxHp_(maxHp),
      mana_(mana),
      maxMana_(maxMana),
      tileSize_(tileSize),
      dragged_(false) {
    setAcceptedMouseButtons(Qt::LeftButton);
    setFlag(QGraphicsItem::ItemIsFocusable, true);
    setCursor(QCursor(Qt::OpenHandCursor));
}

int UnitGraphicsItem::unitId() const { return unitId_; }

void UnitGraphicsItem::setStats(int hp, int maxHp, int mana, int maxMana) {
    hp_ = hp;
    maxHp_ = maxHp;
    mana_ = mana;
    maxMana_ = maxMana;
    update();
}

QPointF UnitGraphicsItem::dragStartScenePos() const { return dragStartScenePos_; }

QRectF UnitGraphicsItem::boundingRect() const {
    const double side = tileSize_ * 0.82;
    return QRectF(0.0, 0.0, side, side);
}

void UnitGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    const QRectF r = boundingRect();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(QColor("#222222")));
    painter->setBrush(QBrush(QColor("#4A90D9")));
    painter->drawRoundedRect(r, 8.0, 8.0);

    const double hpRatio = (maxHp_ > 0) ? static_cast<double>(hp_) / static_cast<double>(maxHp_) : 0.0;
    const double manaRatio = (maxMana_ > 0) ? static_cast<double>(mana_) / static_cast<double>(maxMana_) : 0.0;

    painter->setBrush(QBrush(QColor("#43A047")));
    painter->setPen(Qt::NoPen);
    painter->drawRect(QRectF(2.0, 2.0, (r.width() - 4.0) * hpRatio, 4.0));

    painter->setBrush(QBrush(QColor("#1E88E5")));
    painter->drawRect(QRectF(2.0, r.height() - 6.0, (r.width() - 4.0) * manaRatio, 4.0));

    painter->setPen(QPen(QColor("#FFFFFF")));
    QFont font = painter->font();
    font.setBold(true);
    painter->setFont(font);
    const QString shortName = name_.left(2).toUpper();
    painter->drawText(r, Qt::AlignCenter, shortName);
}

void UnitGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    dragged_ = false;
    dragStartScenePos_ = scenePos();
    grabOffset_ = event->pos();
    setCursor(QCursor(Qt::ClosedHandCursor));
    setZValue(100.0);
    QGraphicsObject::mousePressEvent(event);
}

void UnitGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    dragged_ = true;
    const QPointF topLeft = event->scenePos() - grabOffset_;
    setPos(topLeft);
    emit dragMoved(unitId_, event->scenePos());
    QGraphicsObject::mouseMoveEvent(event);
}

void UnitGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    setCursor(QCursor(Qt::OpenHandCursor));
    setZValue(10.0);
    if (!dragged_) {
        emit unitClicked(unitId_);
    } else {
        emit dragFinished(unitId_, event->scenePos());
    }
    QGraphicsObject::mouseReleaseEvent(event);
}

}  // namespace ui
}  // namespace my_auto_arena
