#ifndef TRACK_H
#define TRACK_H

#include <QGraphicsItem>
#include <QGraphicsView>
#include <QBrush>
#include <QPen>
#include <QPainter>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>

enum class HoverState {
    None,
    Left,
    Right
};

enum class EditMode {
    None,
    Move,
    ResizeLeft,
    ResizeRight
};

class Track : public QGraphicsItem
{
public:
    Track(int _length, QColor _color, QGraphicsScene *_scene){
        setFlags(ItemIsMovable);
        setFlag(ItemIsSelectable);
        setAcceptHoverEvents(true);
        setFlag(QGraphicsItem::ItemSendsGeometryChanges);
        setFlags(flags() | QGraphicsItem::ItemIsMovable);
        color = _color;
        outlineColor = color.lighter(30);
        brush = QBrush(color);
        pen = QPen(outlineColor, penWidth);
        pen.setCapStyle(Qt::RoundCap);
        length = _length;
        height = 30;
        oldPos = scenePos();
        scene = _scene;
        zoom = 1.0;
    }

    void SetLength(float _length){
        length = _length;
        update();
    }

    QColor color;
    QColor outlineColor;
    static QColor selectedColor;
    static QColor selectedColorOutline;
    static int penWidth;
    static int rounded;
    static bool hasShadow;
    static float thresholdShadow;
    QBrush brush;
    QPen pen;
    int length, oldLength, height;
    EditMode mode = EditMode::None;
    HoverState hoverState = HoverState::None;
    QPointF oldPos,oldMousePos;
    QGraphicsScene *scene;
    float zoom;
    // QGraphicsItem interface
public:
    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void setZoomLevel(float zoom);
    // QGraphicsItem interface
protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    // QGraphicsItem interface
protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

};

#endif // TRACK_H
