#ifndef INDICATOR_H
#define INDICATOR_H

#include <QGraphicsItem>
#include <QBrush>
#include <QPen>
#include <QPainter>
#include <QLine>
#include <QPolygonF>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QVector>
#include <QGraphicsScene>
#include <QTimer>
#include <QCursor>
#include <QPointF>
#include <QDebug>
#include <QGraphicsItemAnimation>
#include <QTimeLine>
#include <QGraphicsSceneMouseEvent>
#include <QObject>

class Indicator : public QGraphicsItem
{

public:
    Indicator(float height, QTimeLine *timer);

    QVector<QPointF> points;
    QBrush brush;
    QPen pen;

    QLine line;//indicator line
    QPolygonF poly;//indicator head polygon
    bool pressed=false;
    // QGraphicsItem interface
public:
    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    // QGraphicsItem interface
    void setHeight(float height);
protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    // QGraphicsItem interface
protected:
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

    // QGraphicsItem interface
protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    QTimeLine *mytimer;
};

#endif // INDICATOR_H
