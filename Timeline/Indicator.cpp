#include "Indicator.h"
#include "qgraphicsscene.h"

Indicator::Indicator(float height):QGraphicsItem ()
{
    pen = QPen(Qt::white,2);
    brush = QBrush(Qt::RoundCap);
    brush.setColor(QColor("#50f"));
    points<<QPointF(-10,-20)<<QPointF(0,0)<<QPointF(10,-20);
    setHeight(height);
    setAcceptHoverEvents(true);
    this->setAcceptDrops(true);

    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsFocusable);
    setFlag(ItemSendsGeometryChanges);
}

QRectF Indicator::boundingRect() const
{
    return QRectF(-10,-20, 20, 220 + 20);
}

void Indicator::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setPen(pen);
    painter->drawLine(line);
    painter->setBrush(brush);
    painter->drawPolygon(points);
}

void Indicator::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->type() == QEvent::MouseButtonPress){
        pressed = true;
        setCursor(Qt::ClosedHandCursor);
    }else if(event->type() == QEvent::MouseButtonRelease){
        pressed = false;
        setCursor(Qt::OpenHandCursor);
    }
    QGraphicsItem::mousePressEvent(event);
    update();
}

void Indicator::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF pos = event->scenePos();
    if(pressed){
        this->setPos(pos.x(),y());  
    }
    QGraphicsItem::mouseMoveEvent(event);
    update();
}

void Indicator::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    pressed = false;
    QGraphicsItem::mouseReleaseEvent(event);
    update();
}

void Indicator::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    setCursor(Qt::OpenHandCursor);
}

void Indicator::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    setCursor(Qt::ArrowCursor);
}


QVariant Indicator::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange && scene()) {
        QPointF newPos = value.toPointF();
        newPos.setY(y());
        if(newPos.x() < 0){
            newPos.setX(0);
        }
        return newPos;
    }
    //scene()->update();
    return QGraphicsItem::itemChange(change, value);
}
