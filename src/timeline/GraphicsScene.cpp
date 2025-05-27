
#include "GraphicsScene.h"
void GraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    //emit sendSceneMouseEventSignal(event);
    onlyPress = true;
    QGraphicsScene::mousePressEvent(event);
}

void GraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (onlyPress) {
        emit sendSceneMouseEventSignal(event);
    }

    QGraphicsScene::mouseReleaseEvent(event);
}

void GraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    //emit sendSceneMouseEventSignal(event);
    onlyPress = false;
    QGraphicsScene::mouseMoveEvent(event);
}

void GraphicsScene::wheelEvent(QGraphicsSceneWheelEvent *event){
    emit sendMouseWheelEventSignal(event);
    //GraphicsScene::wheelEvent(event);
}
