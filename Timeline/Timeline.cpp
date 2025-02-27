#include "Timeline.h"
#include "EventProcessor.h"
#include "GridGraphicsGroup.h"
#include "Track.h"
#include <cmath>

int Track::penWidth = 4;
bool Track::hasShadow = false;
float Track::thresholdShadow=0.0f;
int Track::rounded = 10;
bool TimeLine::snapToGrid = true;


TimeLine::TimeLine(EventProcessor *ep, QWidget *_parent)
{
    eventProcessor = ep;
    scene = new GraphicsScene();
    view = new GraphicsView(scene);
    view->setOptimizationFlag(QGraphicsView::IndirectPainting, true);
    view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    view->setDragMode(QGraphicsView::RubberBandDrag);
    view->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    view->setInteractive(true);
    view->setMouseTracking(true);
    view->setFocusPolicy(Qt::StrongFocus);
    view->setFocus(Qt::TabFocusReason);

    connect(view, &GraphicsView::sendKeyPressSignal, this, &TimeLine::keyPressEvent);
    connect(scene, &GraphicsScene::sendSceneMouseEventSignal, this, &TimeLine::mousePressEvent);

    qDebug() << "New Timeline";
    grid = new QGraphicsItemGroup();
    followTime = false;
    zoom = 1.0;
    bpm = 100;
    bars = 400;
    gridSnapInterval = 4;
    selected = new QList<Track *>();
    clipboardGroup = nullptr;
    selectionGroup = nullptr;

    connect(scene, &GraphicsScene::sendMouseWheelEventSignal, this, &TimeLine::onZoom);
    connect(this, &TimeLine::setTime, this, &TimeLine::setTimeImpl);
    setMouseTracking(true);

    drawGrid();

    timer = new QTimeLine(500000);
    indicator = new Indicator(220, timer);
    scene->addItem(indicator);
    indicator->setZValue(101);
    indicator->show();


    timer->setEasingCurve(QEasingCurve::Linear);
    animation = new QGraphicsItemAnimation;
    animation->setItem(indicator);
    animation->setTimeLine(timer);
    connect(timer, &QTimeLine::finished, this, &TimeLine::timerFinished);

    startAnimation();

    connect(view, &QGraphicsView::rubberBandChanged, this, &TimeLine::rubberBandChanged);

    QTimer *qtimer = new QTimer(this);
    connect(qtimer, &QTimer::timeout, this, &TimeLine::checkTime);
    qtimer->start(10);
}

TimeLine::~TimeLine()
{
}

void TimeLine::mousePressEvent(QGraphicsSceneMouseEvent *event)
{

    auto presseditem = this->scene->itemAt(event->scenePos(), QTransform());
    QGraphicsItem* group = dynamic_cast<QGraphicsItemGroup *>(presseditem);
    QGraphicsItem* gridgroup = dynamic_cast<GridGraphicsGroup *>(presseditem);
    QGraphicsItem* track = dynamic_cast<Track *>(presseditem);

    if(!presseditem || gridgroup || !group && track && !track->group()) {

        if(!selectionGroup) {
            bool ok;
            QString text = QInputDialog::getText(this, tr("QInputDialog::getText()"),
                                                 tr("User name:"), QLineEdit::Normal,
                                                 "", &ok);
            if (ok && !text.isEmpty()) {
                addItem(event->scenePos().x() / zoom / barResolution, 1.0, event->scenePos().y() / 35, text.toStdString(), QColor(0,255,0));
            }
        }

        clearClipboard();
        clearSelection();
    }
}

void TimeLine::startAnimation() {
    timer->start();
}

void TimeLine::timerFinished()
{
    qDebug()<< "Timer finished";
    timer->start();
}

float TimeLine::barWidth() {
    return 100 * (60.0 / bpm);
}

void TimeLine::drawGrid(float barWidth) {
    if (grid->childItems().size() > 0) {
        scene->removeItem(grid);
        grid = new GridGraphicsGroup();
    }
    // Side "lanes"
    for(int i = 0; i < 10; i++){
        QString text = QString(std::to_string(i).c_str());
        AlignedTextItem *textitem = new AlignedTextItem(grid);
        textitem->setText(text);
        textitem->setAligment(Qt::AlignLeft);
        textitem->setDefaultTextColor(Qt::white);
        textitem->setBoundingRect(-100, i * 35, 100, 50);
        grid->addToGroup(textitem);
        QGraphicsLineItem *lineitem = new QGraphicsLineItem(-100, (i + 1) * 35, 0, (i + 1) * 35);
        lineitem->setPen(QPen(Qt::white));
        grid->addToGroup(lineitem);
    }

    QGraphicsLineItem *lineitem = new QGraphicsLineItem(0, 0, 0, 350);
    lineitem->setPen(QPen(Qt::white));
    grid->addToGroup(lineitem);

    QFont font = scene->font();
    QFontMetricsF fontMetrics(font);
    int fontWidth = fontMetrics.boundingRect("000").width();

    if(fontWidth > barWidth) {
        font.setPointSize(4);
    }

    for(int i = 0; i < bars; i++){
        if(barWidth > 20) {
            QString text = QString::number(i);
            AlignedTextItem *textitem = new AlignedTextItem(grid);
            textitem->setText(text);
            textitem->setFont(font);
            textitem->setDefaultTextColor(Qt::white);
            textitem->setBoundingRect(i*barWidth, -100, barWidth, 50);
            grid->addToGroup(textitem);
        }

        QGraphicsLineItem *lineitem = new QGraphicsLineItem(i * barWidth, - 100, i * barWidth, -50);
        lineitem->setPen(QPen(Qt::white));
        grid->addToGroup(lineitem);

        QGraphicsLineItem *lineitem2 = new QGraphicsLineItem(i * barWidth, 0, i * barWidth, 350);
        lineitem2->setPen(QPen(QColor(100,100,100)));
        grid->addToGroup(lineitem2);
    }

    QGraphicsLineItem *line1 = new QGraphicsLineItem(0, -50, bars * barWidth, -50);
    line1->setPen(QPen(Qt::white));
    QGraphicsLineItem *line2 = new QGraphicsLineItem(0, 0, bars * barWidth, 0);
    line2->setPen(QPen(Qt::white));
    grid->addToGroup(line1);
    grid->addToGroup(line2);
    grid->setZValue(-10);
    scene->addItem(grid);
}

void TimeLine::setTimeImpl(double time) {
    timer->setPaused(true);
    timer->setCurrentTime((int)(time * 1000.0));
    if (!paused) {
        timer->setPaused(false);
    }
}

void TimeLine::checkTime()
{

    std::list<EventModel*>::iterator i = activeEvents.begin();
    while(i != activeEvents.end()) {
        if ((*i)->start + (*i)->duration < (float)timer->currentTime() / 1000.0) {
            eventProcessor->textEvent(" ");
            i = activeEvents.erase(i);
            continue;
        }
        i++;
    }

    if(events.size() > 0 && nextEvent != events.end() && (*nextEvent)->start < (float)timer->currentTime() / 1000.0) {
        activeEvents.push_back(*nextEvent);
        eventProcessor->textEvent((*nextEvent)->text);
        nextEvent++;
    }


    if (followTime) {
        view->centerOn(indicator->scenePos());
    }
}

int TimeLine::getGridSnapInterval() const
{
    return gridSnapInterval;
}

void TimeLine::setGridSnapInterval(int newGridSnapInterval)
{
    gridSnapInterval = newGridSnapInterval;
}

std::vector<Track *> TimeLine::getTracks() const
{
    return tracks;
}

void TimeLine::buildEventList() {
    activeEvents.clear();
    events.clear();
    for (Track * t : tracks) {
        EventModel *m = new EventModel(t);
        events.push_back(m);
    }
    std::sort(events.begin(),
              events.end(),
              [](const EventModel * lhs, const EventModel * rhs)
              {
                  return lhs->start < rhs->start;
              });
    nextEvent = events.begin();

    while(events.size() > 0 && nextEvent != events.end() && (*nextEvent)->start < (float)timer->currentTime() / 1000.0) {
        nextEvent++;
    }
}

int TimeLine::getBpm() const
{
    return bpm;
}

double TimeLine::getZoom() const
{
    return zoom;
}

void TimeLine::setPaused(bool newPaused)
{
    paused = newPaused;
    timer->setPaused(paused);
}

void TimeLine::setBpm(int newBpm)
{
    bpm = newBpm;
    update();
}

bool TimeLine::getFollowTime() const
{
    return followTime;
}

void TimeLine::setFollowTime(bool newFollowTime)
{
    followTime = newFollowTime;
}

void TimeLine::clear() {
    for (Track *t: tracks) {
        scene->removeItem(t);
    }
    tracks.clear();
}

void TimeLine::onZoom(QGraphicsSceneWheelEvent *event) {
    if (event->orientation() == Qt::Horizontal) {
        view->horizontalScrollBar()->setValue(view->horizontalScrollBar()->value() - event->delta());
    } else {

        auto oldCenter = view->mapToScene(view->viewport()->rect().center()) / zoom;
        zoom += (float)event->delta() / 1000.0f;
        zoom = std::clamp(zoom, 0.1, 10.0);

        for (Track * track : tracks) {
            track->updatePosition();
        }
        if (selectionGroup) {
            selectionGroup->setZoomLevel(zoom);
            selectionGroup->update();
        }
        view->horizontalScrollBar()->setMaximum(barWidth() * zoom * (bars + 1) - view->width());
        view->centerOn(oldCenter * zoom);

        indicator->setX((timer->currentTime() / 1000.0) * barResolution * zoom);
        update();
    }
}

void TimeLine::addItem(float start, float length, int lane, std::string text,  QColor color)
{
    Track *track = new Track(start, length, lane, color, QString(text.c_str()), this);
    tracks.push_back(track);
    scene->addItem(track);
}

void TimeLine::updateAnimation() {
    animation->clear();
    for (float i = 0.0; i < bars; i+=0.1)
        animation->setPosAt((float)i / bars, QPointF(i * barWidth() * zoom, 0));
}

void TimeLine::clearSelection() {
    if (selectionGroup) {
        auto items = selectionGroup->childItems();
        scene->destroyItemGroup(selectionGroup);
        for (auto item : items) {
            item->setSelected(false);
        }
        selectionGroup = nullptr;
    }
}

void TimeLine::clearClipboard() {
    if (clipboardGroup) {
        auto items = clipboardGroup->childItems();
        scene->destroyItemGroup(clipboardGroup);
        for (auto item : items) {
            item->setSelected(false);
        }
        clipboardGroup = nullptr;
    }
}

void TimeLine::rubberBandChanged(QRect viewportRect, QPointF fromScenePoint, QPointF toScenePoint)
{
    if (viewportRect.isNull()) {
        qDebug() << "Creating selection";
        selectionGroup = new TrackGroup(this->scene->activePanel());
        selectionGroup->setZoomLevel(zoom);
        selectionGroup->setFlag(QGraphicsItem::ItemIsSelectable);
        for (Track * t : *selected) {
            selectionGroup->addToGroup(t);
            t->setParentItem(selectionGroup);
            t->setSelected(true);
        }
        selectionGroup->update();
        scene->addItem(selectionGroup);
        return;
    }

    clearClipboard();
    clearSelection();

    QRectF area = fromScenePoint.x() < toScenePoint.x() || fromScenePoint.y() < toScenePoint.y() ? QRectF(fromScenePoint, toScenePoint) : QRectF(toScenePoint, fromScenePoint);
    selected->clear();
    QList<QGraphicsItem *> selection = this->scene->items(area);
    for (auto item : selection) {
        auto* track = dynamic_cast<Track *>(item);
        if (track) {
            selected->push_back(track);
        }
    }
}

void TimeLine::setTrackTime(float time)
{
    if (trackTime != time) {
        timer->setDuration(time * 1000.0);
        trackTime = time;
        bars = ceil(trackTime) * (bpm / 60.0);
        update();
    }
}

void TimeLine::update(const QRectF &rect) {
    bars = ceil(trackTime) * (bpm / 60.0);
    drawGrid(barWidth() * zoom);
    updateAnimation();
}

void TimeLine::keyPressEvent(QKeyEvent * event) {
    switch (event->key()) {
    case (Qt::Key_Delete):

        if(selectionGroup){
            for (auto item : selectionGroup->childItems()) {
                auto itr = std::remove_if(tracks.begin(),tracks.end(), [&](Track* a){return a == (Track *)item;});
                tracks.erase(itr,tracks.end());
            }
            scene->removeItem(selectionGroup);
        } else {

        }
        qDebug() << tracks.size();
        break;

    case (Qt::Key_C):
        if (event->modifiers()==Qt::ControlModifier){
            clipboardGroup = new TrackGroup();
            clipboardGroup->setZoomLevel(zoom);
            clipboardGroup->setFlag(QGraphicsItem::ItemIsSelectable);
            for (Track * t : *selected) {
                Track * clone = t->clone();
                clone->setParentItem(clipboardGroup);
                clipboardGroup->addToGroup(clone);
                clone->setSelected(true);
            }
        }
        break;
    case (Qt::Key_V):
        if (event->modifiers()==Qt::ControlModifier){
            if (clipboardGroup) {
                clearSelection();
                clipboardGroup->setX(view->mapToScene(view->viewport()->rect().center()).x() / zoom);
                clipboardGroup->update();
                scene->addItem(clipboardGroup);
                for (auto item : clipboardGroup->childItems()) {
                    ((Track *)item)->calculateStartTime();
                    tracks.push_back((Track *)item);
                }
            }
        }
        break;
    }
}

