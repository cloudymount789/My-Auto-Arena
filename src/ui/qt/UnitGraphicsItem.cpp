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
                                   double tileSize, bool isEnemy, int starLevel)
    : unitId_(unitId),
      name_(name),
      hp_(hp),
      maxHp_(maxHp),
      mana_(mana),
      maxMana_(maxMana),
      tileSize_(tileSize),
      isEnemy_(isEnemy),
      starLevel_(starLevel),
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

void UnitGraphicsItem::setStarLevel(int starLevel) {
    starLevel_ = starLevel;
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
    // 玩家单位：蓝色；敌方单位：红色，方便玩家区分阵营。
    const QColor bodyColor = isEnemy_ ? QColor("#D94A4A") : QColor("#4A90D9");
    painter->setBrush(QBrush(bodyColor));
    painter->drawRoundedRect(r, 8.0, 8.0);

    const double hpRatio = (maxHp_ > 0) ? static_cast<double>(hp_) / static_cast<double>(maxHp_) : 0.0;
    const double manaRatio = (maxMana_ > 0) ? static_cast<double>(mana_) / static_cast<double>(maxMana_) : 0.0;

    painter->setBrush(QBrush(QColor("#43A047")));
    painter->setPen(Qt::NoPen);
    painter->drawRect(QRectF(2.0, 2.0, (r.width() - 4.0) * hpRatio, 4.0));

    painter->setBrush(QBrush(QColor("#1E88E5")));
    painter->drawRect(QRectF(2.0, r.height() - 6.0, (r.width() - 4.0) * manaRatio, 4.0));

    // 绘制单位名称缩写（居中）。
    painter->setPen(QPen(QColor("#FFFFFF")));
    QFont font = painter->font();
    font.setBold(true);
    font.setPixelSize(14);
    painter->setFont(font);
    const QString shortName = name_.left(2).toUpper();
    // 将名称绘制在中上区域，为星级留出下方空间。
    const QRectF nameRect(0.0, r.height() * 0.2, r.width(), r.height() * 0.45);
    painter->drawText(nameRect, Qt::AlignCenter, shortName);

    // 在名称下方绘制星级符号（★）。
    if (starLevel_ > 0 && !isEnemy_) {
        QFont starFont = painter->font();
        starFont.setBold(false);
        starFont.setPixelSize(10);
        painter->setFont(starFont);
        painter->setPen(QPen(QColor("#FFD700")));
        QString stars;
        for (int i = 0; i < starLevel_; ++i) {
            stars += QString::fromUtf8("\xe2\x98\x85");  // UTF-8 编码的 ★
        }
        const QRectF starRect(0.0, r.height() * 0.65, r.width(), r.height() * 0.28);
        painter->drawText(starRect, Qt::AlignCenter, stars);
    }
}

void UnitGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() != Qt::LeftButton) {
        QGraphicsObject::mousePressEvent(event);
        return;
    }
    // 必须 accept，否则场景不会把本项当作 mouse grabber，后续收不到 mouseMoveEvent（拖拽失效）。
    event->accept();
    dragged_ = false;
    dragStartScenePos_ = scenePos();
    grabOffset_ = event->pos();
    setCursor(QCursor(Qt::ClosedHandCursor));
    setZValue(100.0);
}

void UnitGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    if (!(event->buttons() & Qt::LeftButton)) {
        QGraphicsObject::mouseMoveEvent(event);
        return;
    }
    event->accept();
    dragged_ = true;
    const QPointF topLeft = event->scenePos() - grabOffset_;
    setPos(topLeft);
    emit dragMoved(unitId_, event->scenePos());
}

void UnitGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        event->accept();
        setCursor(QCursor(Qt::OpenHandCursor));
        setZValue(10.0);
        if (!dragged_) {
            emit unitClicked(unitId_);
        } else {
            emit dragFinished(unitId_, event->scenePos());
        }
        return;
    }
    QGraphicsObject::mouseReleaseEvent(event);
}

}  // namespace ui
}  // namespace my_auto_arena
