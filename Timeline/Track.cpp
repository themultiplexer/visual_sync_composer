#include "Track.h"

QRectF Track::boundingRect()const {
    return QRectF(0,0,length * zoom, height);
}
void Track::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){

    Q_UNUSED(widget);
    if (isSelected()) {
        painter->setBrush(brush);
    }

    painter->setPen(pen);
    painter->drawRoundedRect(boundingRect(),rounded,rounded);
    painter->setBrush(outlineColor);

    QFont font=scene->font();
    QFontMetricsF fontMetrics(font);
    QString text("tReplaySong1");
    int heightFont = fontMetrics.boundingRect(text).height();
    painter->drawText(10, -5, length * zoom - 20, heightFont, Qt::AlignBaseline, text);
}

void Track::mousePressEvent(QGraphicsSceneMouseEvent *event){
    switch (hoverState) {
    case HoverState::None:
        mode = EditMode::Move;
        setCursor(Qt::ClosedHandCursor);
        this->setZValue(101);
        break;
    case HoverState::Left:
        mode = EditMode::ResizeLeft;
        break;
    case HoverState::Right:
        mode = EditMode::ResizeRight;
        break;
    default:
        break;
    }

    oldMousePos = event->scenePos();
    oldPos = scenePos() - QPointF(transform().dx(), transform().dy());
    oldLength = length;
}

void Track::mouseMoveEvent(QGraphicsSceneMouseEvent *event){
    prepareGeometryChange();
    switch (mode) {
        case EditMode::Move:
            {
                QPointF newPos = event->scenePos();
                int yDiff = newPos.y() - oldPos.y();
                int dx = (newPos - oldMousePos).x() / zoom;

                // Height snapping to grid
                int heightDiff=15;
                if(oldPos.x() + dx < 0) {
                    newPos.setX(0);
                    setX(newPos.x());
                } else if(newPos.y() < 0) {
                    newPos.setY(0);
                    setY(newPos.y());
                    setX(oldPos.x() + dx);
                } else if (abs(yDiff) > heightDiff){
                    heightDiff*=2;
                    heightDiff+=5;
                    int d = (int)(yDiff%heightDiff);
                    newPos.setY(oldPos.y()+(int)(yDiff/heightDiff)*heightDiff);//*((int)(yDiff/30))>0?1:0);
                    setY(newPos.y());
                    setX(oldPos.x()+dx);
                } else {
                    setY(oldPos.y());
                    setX(oldPos.x()+dx);
                }

                QTransform transform;
                transform.translate(zoom * pos().x() - pos().x(), 0);
                setTransform(transform);
            }
            break;
        case EditMode::ResizeLeft:
            {
                int dx = (event->scenePos() - oldMousePos).x() / zoom;
                if (oldLength - dx > 0) {
                    if(oldPos.x()+dx >= 0) {
                        setX(oldPos.x()+dx);
                        QTransform transform;
                        transform.translate(zoom * pos().x() - pos().x(), 0);
                        setTransform(transform);
                        SetLength(oldLength - dx);
                    } else {
                        setX(0);
                        QTransform transform;
                        transform.translate(zoom * pos().x(), 0);
                        setTransform(transform);
                        SetLength(oldLength + oldMousePos.x() / zoom);
                    }
                }
            }
            break;
        case EditMode::ResizeRight:
                if (event->pos().x() > 0) {
                    SetLength(event->pos().x() / zoom);
                }

            break;
        default:
            break;
    }
}

void Track::setZoomLevel(float zoom) {
    this->zoom = zoom;
    resetTransform();
    QTransform transform;
    transform.translate(zoom * pos().x() - pos().x(), 0);
    setTransform(transform);
}

void Track::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{

}

void Track::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    QPointF posInItem = event->pos();

    if (posInItem.x() > length * zoom - 10) {
        setCursor(Qt::SizeHorCursor);
        hoverState = HoverState::Right;
    } else if (posInItem.x() < 10) {
        setCursor(Qt::SizeHorCursor);
        hoverState = HoverState::Left;
    } else {
        setCursor(Qt::OpenHandCursor);
        hoverState = HoverState::None;
    }
}

void Track::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    setCursor(Qt::ArrowCursor);
    hoverState = HoverState::None;
}

void Track::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    mode = EditMode::None;
    oldMousePos = event->scenePos();
    oldPos = scenePos();
    this->setZValue(0);
    setCursor(Qt::ArrowCursor);
}
void Track::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event){
    qDebug()<<"Double Click";
}



QVariant Track::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedChange)
    {
        qDebug() << "Changed";
        if (value == true)
        {
            //brush.setColor(QColor(255,255,255));
        }
        else
        {
            //outlineColor = QColor(255,255,255);
        }
        update();
    }

    return QGraphicsItem::itemChange(change, value);
}


