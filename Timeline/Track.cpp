#include "Track.h"
#include <cmath>
#include <iomanip>

float Track::length() const {
    return duration * 100 * timeline->getZoom();
}

void Track::setDuration(float newDuration)
{
    duration = newDuration;
}

QRectF Track::boundingRect() const {
    return QRectF(0, 0, length(), height);
}
void Track::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){

    Q_UNUSED(widget);
    if (isSelected()) {
        painter->setBrush(brush);
    }

    painter->setPen(pen);
    painter->drawRoundedRect(boundingRect(), rounded, rounded);
    painter->setBrush(outlineColor);

    QFont font=timeline->scene->font();
    QFontMetricsF fontMetrics(font);

    if (isSelected()) {
        painter->setPen(QColor(100, 100, 100));
    }

    int heightFont = fontMetrics.boundingRect(text).height();
    painter->drawText(10, -5, length() - 20, heightFont, Qt::AlignBaseline, text);
}

void Track::update(const QRectF &rect) {
    /*
    std::stringstream s;
    s << " " << std::fixed << std::showpoint << std::setprecision(2) << startTime  << " " << duration;
    text = QString(s.str().c_str());
    */
}

void Track::updatePosition() {
    setX(startTime * TimeLine::barResolution * timeline->getZoom());
    update();
}

void Track::calculateStartTime(){
    startTime = scenePos().x() / TimeLine::barResolution / timeline->getZoom();
    qDebug() << "Setting start time: " << startTime;
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
    oldPos = scenePos();
    oldLength = length();
}

void Track::mouseMoveEvent(QGraphicsSceneMouseEvent *event){
    prepareGeometryChange();
    switch (mode) {
        case EditMode::Move:
            {
                QPointF newPos = event->scenePos();
                int yDiff = newPos.y() - oldPos.y();
                int dx = (newPos - oldMousePos).x();

                newPos = oldPos;

                // Height snapping to lane
                int heightDiff=15;
                if (abs(yDiff) > heightDiff){
                    // Snap to y lanes
                    heightDiff*=2;
                    heightDiff+=5;
                    newPos.setY(oldPos.y()+(int)(yDiff/heightDiff)*heightDiff);
                }
                // Free Moving
                if(TimeLine::snapToGrid) {
                    setY(oldPos.y());
                    qreal newx = (oldPos.x()+dx);
                    float gridWidth = timeline->barWidth() * timeline->getZoom();
                    gridWidth /= 2.0f; // TODO make grid snap intervals
                    qreal snappedX = round(newx/gridWidth) * gridWidth;
                    qreal distance = std::abs(newx - snappedX);

                    if (distance <= 25.0) {
                        newPos.setX(snappedX);
                    } else {
                        newPos.setX(newx);
                    }
                } else {
                    newPos.setX(oldPos.x()+dx);
                }

                if(oldPos.x() + dx < 0) {
                    // Snap to left
                    newPos.setX(0);
                }
                if(newPos.y() < 0) {
                    // Snap to top
                    newPos.setY(0);
                }

                setPos(newPos);

            }
            break;
        case EditMode::ResizeLeft:
            {
                int dx = (event->scenePos() - oldMousePos).x();
                if (oldLength - dx > 0) {
                    if(oldPos.x()+dx >= 0) {
                        setX(oldPos.x()+dx);
                        setDuration((float)(oldLength - dx) / timeline->getZoom() / TimeLine::barResolution);
                    } else {
                        setX(0);
                        setDuration((oldLength + oldMousePos.x()) / timeline->getZoom() / TimeLine::barResolution);
                    }
                }
            }
            break;
        case EditMode::ResizeRight:
            if (event->pos().x() > 0) {
                setDuration(event->pos().x() / timeline->getZoom() / TimeLine::barResolution);
            }
            break;
        default:
            break;
    }
    update();
}

void Track::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{

}

void Track::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    QPointF posInItem = event->pos();

    if (posInItem.x() > length() - 10) {
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
    bool wasChanged = oldPos != scenePos() || length() != oldLength;
    oldPos = scenePos();
    calculateStartTime();
    this->setZValue(0);
    setCursor(Qt::ArrowCursor);
    update();

    if (wasChanged) {
        qDebug("Changed");
        timeline->buildEventList();
    }
}
void Track::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event){
    qDebug()<<"Double Click";
}

QVariant Track::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange) {
        //startTime = scenePos().x() / zoom;
    }

    return QGraphicsItem::itemChange(change, value);
}

