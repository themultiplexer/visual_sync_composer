#include "Indicator.h"

Indicator::Indicator(float height, QTimeLine *timer):QGraphicsItem ()
{
    mytimer = timer;
    pen = QPen(Qt::white,2);
    brush = QBrush(Qt::RoundCap);
    brush.setColor(QColor(0,0, 200));
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

    painter->drawText(10, -30, 100, 50, Qt::AlignBaseline, QString(std::to_string(mytimer->currentTime() / 1000.0).c_str()));
}

void Indicator::setHeight(float height) {
    line.setP2(QPoint(0,height));
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

    }
    //scene()->update();
    return QGraphicsItem::itemChange(change, value);
}
