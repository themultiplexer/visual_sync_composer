#ifndef TRACK_H
#define TRACK_H

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

    Track(float start, int _length, int lane, QColor _color, QGraphicsScene *_scene){
        setFlags(ItemIsMovable);
        setFlag(ItemIsSelectable);
        setAcceptHoverEvents(true);
        setFlag(QGraphicsItem::ItemSendsGeometryChanges);
        setFlags(flags() | QGraphicsItem::ItemIsMovable);
        zoom = 1.0;
        startTime = start;
        color = _color;
        outlineColor = color;
        brush = QBrush(color);
        pen = QPen(outlineColor, penWidth);
        pen.setCapStyle(Qt::RoundCap);
        length = _length;
        height = 30;

        setPos(QPointF(start * zoom, lane * 35));

        oldPos = scenePos();
        scene = _scene;


        text = QString("tReplaySong1");
    }

    static Track * fromTrack(const Track *originalTrack){
        return new Track(originalTrack->startTime, originalTrack->length, (int)originalTrack->pos().y() / 35, originalTrack->color, originalTrack->scene);
    }

    Track* clone() const { return Track::fromTrack(this); }

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
    float startTime;
    int length, oldLength, height;
    EditMode mode = EditMode::None;
    HoverState hoverState = HoverState::None;
    QPointF oldPos,oldMousePos;
    QGraphicsScene *scene;
    float zoom;
    QString text;
    // QGraphicsItem interface
public:
    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void setZoomLevel(float zoom);

    // QGraphicsItem interface
    void setStartTime(float newStartTime);
    void update(const QRectF &rect = QRectF());

    void calculateStartTime();
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
};

#endif // TRACK_H
