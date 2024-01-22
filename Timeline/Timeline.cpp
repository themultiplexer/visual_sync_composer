#include "Timeline.h"
#include "AlignedTextItem.h"

TimeLine::TimeLine(QWidget *parent) :
    QWidget(parent)
{

}

int Track::penWidth = 4;
bool Track::hasShadow = false;
float Track::thresholdShadow=0.0f;
int Track::rounded = 10;

TimeLine::TimeLine(GraphicsView *_view, QWidget *_parent)
{
    qDebug() << "New Timeline";
    grid = new QGraphicsItemGroup();
    barResolution = 100;
    followTime = false;
    view = _view;
    zoom = 1.0;
    bars = 400;
    connect(view, &GraphicsView::sendMouseWheelEventSignal, this, &TimeLine::onZoom);
    connect(this, &TimeLine::setTime, this, &TimeLine::setTimeImpl);
    scene = view->scene();
    setFocusPolicy(Qt::WheelFocus);
    setMouseTracking(true);

    drawGrid();

    for(int i = 0; i < 5; i++){
        QString text = QString::number(i);
        AlignedTextItem *textitem = new AlignedTextItem(grid);
        textitem->setText(text);
        textitem->setAligment(Qt::AlignLeft | Qt::AlignTop);
        textitem->setDefaultTextColor(Qt::white);
        textitem->setBoundingRect(-100, i * 35, 100, 50);
        QGraphicsLineItem *lineitem = new QGraphicsLineItem(-100, i * 35, 0, i * 35);
        lineitem->setPen(QPen(Qt::white));
        scene->addItem(textitem);
        scene->addItem(lineitem);
    }

    QGraphicsLineItem *lineitem = new QGraphicsLineItem(0, 0, 0, 200);
    lineitem->setPen(QPen(Qt::white));
    scene->addItem(lineitem);

    indicator = new Indicator(220);
    scene->addItem(indicator);
    indicator->setZValue(101);
    indicator->show();

    timer = new QTimeLine(500000);
    timer->setEasingCurve(QEasingCurve::Linear);
    animation = new QGraphicsItemAnimation;
    animation->setItem(indicator);
    animation->setTimeLine(timer);
    timer->setLoopCount(2);
    //connect(timer, &QTimeLine::finished, this, TimeLine::finished);

    startAnimation();

    connect(view, &QGraphicsView::rubberBandChanged, this, &TimeLine::rubberBandChanged);

    QTimer *qtimer = new QTimer(this);
    connect(qtimer, &QTimer::timeout, this, &TimeLine::checkTime);
    qtimer->start(10);

    anim = new QPropertyAnimation(view->horizontalScrollBar(), "value", this);
    anim->setDuration(10000);
    anim->setStartValue(0);
    anim->setEndValue(bars * barResolution);
}

TimeLine::~TimeLine()
{
}

void TimeLine::startAnimation() {
    timer->start();
}

void TimeLine::drawGrid(int xOffset) {
    if (grid->childItems().size() > 0) {
        scene->removeItem(grid);
        grid = new QGraphicsItemGroup();
    }

    QFont font = scene->font();
    QFontMetricsF fontMetrics(font);

    for(int i = 0; i < bars; i++){
        QString text = QString::number(i);
        AlignedTextItem *textitem = new AlignedTextItem(grid);
        textitem->setText(text);
        textitem->setFont(font);
        textitem->setDefaultTextColor(Qt::white);
        textitem->setBoundingRect(i*xOffset, -100, xOffset, 50);
        QGraphicsLineItem *lineitem = new QGraphicsLineItem(i * xOffset, - 100, i * xOffset, -50);
        lineitem->setPen(QPen(Qt::white));
        grid->addToGroup(textitem);
        grid->addToGroup(lineitem);
    }

    QGraphicsLineItem *line1 = new QGraphicsLineItem(0, -50, bars * xOffset, -50);
    line1->setPen(QPen(Qt::white));
    QGraphicsLineItem *line2 = new QGraphicsLineItem(0, 0, bars * xOffset, 0);
    line2->setPen(QPen(Qt::white));
    grid->addToGroup(line1);
    grid->addToGroup(line2);
    scene->addItem(grid);
}

void TimeLine::setTimeImpl(double time) {
    timer->setPaused(true);
    timer->setCurrentTime((int)(time * 1000.0));
    timer->setPaused(false);
    if (anim->state() == QAbstractAnimation::State::Running) {
        anim->setCurrentTime((int)(time * 1000.0));
    }

}

void TimeLine::checkTime()
{

}

bool TimeLine::getFollowTime() const
{
    return followTime;
}

void TimeLine::setFollowTime(bool newFollowTime)
{
    followTime = newFollowTime;
    if (followTime) {
        if (anim->state() == QAbstractAnimation::State::Paused) {
            anim->setPaused(false);
        } else {
            anim->start();
        }

    } else {
        anim->setPaused(true);
    }
}

void TimeLine::Clear() {
    for (Track *t: tracks) {
        scene->removeItem(t);
    }
    tracks.clear();
}

void TimeLine::onZoom(QWheelEvent *event) {
    qDebug() << "On zoom";
    if (event->angleDelta().x() != 0) {
        view->horizontalScrollBar()->setValue(view->horizontalScrollBar()->value() - event->angleDelta().x());
    } else {
        float oldScrollPos = (float)view->horizontalScrollBar()->value() / zoom;
        zoom += (float)event->angleDelta().y() / 1000.0f;
        zoom = std::clamp(zoom, 0.5f, 5.0f);
        drawGrid(barResolution * zoom);
        for (Track * track : tracks) {
            track->setZoomLevel(zoom);
        }
        view->horizontalScrollBar()->setMaximum(barResolution * zoom * bars - view->width());
        view->horizontalScrollBar()->setValue(oldScrollPos * zoom);
        updateAnimation();
    }
}

void TimeLine::AddItem(float start, float length, int lane,  QColor color)
{
    Track *track = new Track(length * barResolution, color, scene);
    tracks.push_back(track);
    scene->addItem(track);
    track->setPos(QPointF(start * barResolution, lane * 35));
}

void TimeLine::updateAnimation() {
    anim->setEndValue(bars * barResolution * zoom - view->width());

    animation->clear();
    for (int i = 0; i < bars; i++)
        animation->setPosAt((float)i / bars, QPointF(i * barResolution * zoom, 0));
}

void TimeLine::rubberBandChanged(QRect viewportRect, QPointF fromScenePoint, QPointF toScenePoint)
{
    QList<QGraphicsItem *> selected = this->scene->items(QRectF(fromScenePoint, toScenePoint));
    for (auto item : selected) {
        auto* track = dynamic_cast<Track *>(item);
        if (track) {
            track->setSelected(true);
        }
    }
}

void TimeLine::setTrackTime(float time)
{
    if (trackTime != time) {
        anim->setDuration(time * 1000.0);
        timer->setDuration(time * 1000.0);
        bars = (int)time;
        trackTime = time;
        updateAnimation();
        timer->setLoopCount(2);
    }
}

std::vector<std::tuple<float, float, int>> TimeLine::Serialize()
{
    std::vector<std::tuple<float, float, int>> times;
    for (Track * t : tracks) {
        auto tu = std::make_tuple(t->scenePos().x() / (float)barResolution, t->length / (float)barResolution, (int)t->scenePos().y() / 35);
        times.push_back(tu);
    }
    return times;
}
