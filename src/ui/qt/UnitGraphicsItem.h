#ifndef MY_AUTO_ARENA_UI_QT_UNIT_GRAPHICS_ITEM_H
#define MY_AUTO_ARENA_UI_QT_UNIT_GRAPHICS_ITEM_H

#include <QGraphicsObject>
#include <QString>

namespace my_auto_arena {
namespace ui {

class UnitGraphicsItem : public QGraphicsObject {
    Q_OBJECT
public:
    UnitGraphicsItem(int unitId, const QString& name, int hp, int maxHp, int mana, int maxMana, double tileSize);

    int unitId() const;
    void setStats(int hp, int maxHp, int mana, int maxMana);
    QPointF dragStartScenePos() const;

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

signals:
    void unitClicked(int unitId);
    void dragMoved(int unitId, QPointF scenePos);
    void dragFinished(int unitId, QPointF releaseScenePos);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    int unitId_;
    QString name_;
    int hp_;
    int maxHp_;
    int mana_;
    int maxMana_;
    double tileSize_;
    QPointF grabOffset_;
    QPointF dragStartScenePos_;
    bool dragged_;
};

}  // namespace ui
}  // namespace my_auto_arena

#endif  // MY_AUTO_ARENA_UI_QT_UNIT_GRAPHICS_ITEM_H
