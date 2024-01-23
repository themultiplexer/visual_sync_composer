#import "TrackGroup.h"
#include "Track.h"
#include <QBrush>
#include <QGraphicsSceneEvent>
#include <QDebug>
#include <QPainter>
#include <QCursor>

TrackGroup::TrackGroup(QGraphicsItem *parent) : QGraphicsItemGroup(parent) {
    setFlags(QGraphicsItem::ItemIsSelectable |
             QGraphicsItem::ItemIsMovable |
             QGraphicsItem::ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);
    bRect = childrenBoundingRect();
    qDebug() << "Setting " << pos();
}

void TrackGroup::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    QBrush brush(QColor(60,60,60));
    //painter->setBrush(brush);
    painter->setPen(QColor(100, 100, 100));
    painter->drawRect(boundingRect());

    painter->drawText(boundingRect().x(), 0, boundingRect().width(), 50, Qt::AlignBaseline, QString("Test"));
}

void TrackGroup::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    qDebug() << "Enter";
    setCursor(Qt::OpenHandCursor);
}

void TrackGroup::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    setCursor(Qt::ArrowCursor);
}

void TrackGroup::update(const QRectF &rect) {
    bRect = childrenBoundingRect();
    oldPos = pos();
    qDebug() << "Setting " << scenePos();
    QGraphicsItemGroup::update(rect);
}

QRectF TrackGroup::boundingRect() const
{
    return bRect;
}

float TrackGroup::getZoomLevel() const
{
    return zoomLevel;
}

void TrackGroup::setZoomLevel(float newZoomLevel)
{
    zoomLevel = newZoomLevel;
    /*
    QTransform transform;
    transform.translate(zoomLevel * boundingRect().x() - boundingRect().x(), 0);
    setTransform(transform);*/
}

void TrackGroup::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    qDebug() << "Item Group Transform:" << transformOriginPoint();
    setCursor(Qt::ClosedHandCursor);
    this->setZValue(101);
    pressed = true;
    oldMousePos = event->scenePos();
    oldPos = pos();
    qDebug() << "Setting " << scenePos();
}

void TrackGroup::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(pressed) {
        // Move whole group
        prepareGeometryChange();
        QPointF newPos = event->scenePos();
        int yDiff = newPos.y() - oldPos.y();
        int dx = (newPos - oldMousePos).x();

        if(bRect.topLeft().x() + dx < 0) {
            newPos.setX(-bRect.topLeft().x());
            setX(newPos.x());
        } else {
            setY(oldPos.y());
            setX(oldPos.x()+dx);
        }

        for (auto item : childItems()) {
            Track *t = (Track *)item;
            t->calculateStartTime();
            t->update();
        }

        /*
        QPointF newPos = event->scenePos();
        int yDiff = newPos.y() - oldPos.y();
        int dx = (newPos - oldMousePos).x();

        for (auto item : childItems()) {
            qDebug() << item->pos() << item->scenePos();
            //((Track *)item)->setStartTime((oldPos.x()+dx / zoom));
        }*/
    }
}

void TrackGroup::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    qDebug() << "Setting " << scenePos();
    pressed = false;
    oldMousePos = event->scenePos();
    oldPos = pos();
    this->setZValue(0);
    setCursor(Qt::ArrowCursor);
}
