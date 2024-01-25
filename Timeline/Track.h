#ifndef TRACK_H
#define TRACK_H

#include "Timeline.h"
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QBrush>
#include <QPen>
#include <QPainter>
#include <QString>
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

    Track(float start, float _duration, int lane, QColor _color, TimeLine *_timeline){
        timeline = _timeline;
        setFlags(ItemIsMovable);
        setFlag(ItemIsSelectable);
        setAcceptHoverEvents(true);
        setFlag(QGraphicsItem::ItemSendsGeometryChanges);
        setFlags(flags() | QGraphicsItem::ItemIsMovable);
        startTime = start;
        duration = _duration;
        color = _color;
        outlineColor = color;
        brush = QBrush(color);
        pen = QPen(outlineColor, penWidth);
        pen.setCapStyle(Qt::RoundCap);
        height = 30;

        setPos(QPointF(start * barResolution * timeline->getZoom(), lane * 35));

        oldPos = scenePos();



        text = QString("tReplaySong1");
    }

    static const int barResolution = 100;

    static Track * fromTrack(const Track *originalTrack){
        return new Track(originalTrack->startTime, originalTrack->duration, (int)originalTrack->pos().y() / 35, originalTrack->color, originalTrack->timeline);
    }

    Track* clone() const { return Track::fromTrack(this); }


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
    float startTime, duration;
    int oldLength, height;
    EditMode mode = EditMode::None;
    HoverState hoverState = HoverState::None;
    QPointF oldPos,oldMousePos;
    TimeLine *timeline;
    QString text;
    // QGraphicsItem interface
public:
    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    // QGraphicsItem interface
    void setStartTime(float newStartTime);
    void update(const QRectF &rect = QRectF());

    void calculateStartTime();
    void setDuration(float newDuration);

    void updatePosition();
protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

private:

    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    float length() const;
};

#endif // TRACK_H
