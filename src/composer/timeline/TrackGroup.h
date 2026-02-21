#ifndef TRACKGROUP_H
#define TRACKGROUP_H

#include <QGraphicsItemGroup>

class TrackGroup : public QGraphicsItemGroup
{
public:
        // QGraphicsItem interface
    float getZoomLevel() const;
    void setZoomLevel(float newZoomLevel);
    void update(const QRectF &rect = QRectF());

    TrackGroup(QGraphicsItem *parent = nullptr);

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
private:
    bool pressed;
    QPointF oldMousePos;
    QPointF oldPos;
    QRectF boundingRect() const override;
    float zoomLevel = 1.0;
    QRectF bRect;
};


#endif // TRACKGROUP_H
