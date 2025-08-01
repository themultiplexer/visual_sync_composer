#ifndef GRAPHSCENE_H
#define GRAPHSCENE_H
#include <QGraphicsScene>
#include <QMouseEvent>
#include <iostream>
#include <QDebug>

class GraphicsScene: public QGraphicsScene
{
    Q_OBJECT
public:
    GraphicsScene(QWidget *parent = nullptr):QGraphicsScene(nullptr){}
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void wheelEvent(QGraphicsSceneWheelEvent *event) override;

signals:
    void sendSceneMouseEventSignal(QGraphicsSceneMouseEvent *event);
    void sendMouseWheelEventSignal(QGraphicsSceneWheelEvent *event);

private:
    bool onlyPress;

};
#endif // GRAPHSCENE_H
