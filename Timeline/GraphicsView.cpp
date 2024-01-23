#include "GraphicsView.h"
#include <QDebug>

void GraphicsView::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);
}

void GraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    auto item = this->scene()->itemAt(event->pos(),QTransform());
    QGraphicsView::mouseMoveEvent(event);
}

void GraphicsView::mouseReleaseEvent(QMouseEvent *event){
    QGraphicsView::mouseReleaseEvent(event);
}

void GraphicsView::keyPressEvent(QKeyEvent *event)
{
    emit sendKeyPressSignal(event);
    QGraphicsView::keyPressEvent(event);
}

void GraphicsView::drawBackground(QPainter *painter, const QRectF &rect){
    QBrush brush(QColor(30,30,30));
    painter->fillRect(rect,brush);
}

void GraphicsView::wheelEvent(QWheelEvent *event){
    emit sendMouseWheelEventSignal(event);
    event->accept();
    //GraphicsView::wheelEvent(event);
}
/*
void GraphicsView::keyPressEvent(QKeyEvent * event) {
    qDebug()<<"event";
    switch (event->key()) {
    case (Qt::Key_C):
        if (event->modifiers()==Qt::ControlModifier)
            qDebug()<<"shift and one";
        break;
    }
    QGraphicsView::keyPressEvent(event);
}
*/
