#ifndef TIMELINE_H
#define TIMELINE_H

#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTransform>
#include <QDebug>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QDebug>
#include <QPen>
#include <QBrush>
#include <QGraphicsItem>
#include <QScrollBar>
#include <QPoint>
#include <QTimer>
#include <tuple>
#include <QGraphicsRectItem>
#include "GraphicsScene.h"
#include "Indicator.h"
#include "GraphicsView.h"
#include "TrackGroup.h"
#include "AlignedTextItem.h"
#include <QInputDialog>
#include "../EventModel.h"
#include <QSpinBox>

class Track;

class TimeLine : public QWidget
{
    Q_OBJECT

public:
    explicit TimeLine(QWidget *parent = nullptr);
    ~TimeLine();
    QGraphicsItem* ItemAt(QPointF position){return scene->itemAt(position,QTransform());}
    void addItem(float start, float length, int lane, std::string text,  QColor color);
    void setTrackTime(float time);
    void clear();
    void startAnimation();
    void timerFinished();
    void updateAnimation();
    void rubberBandChanged(QRect viewportRect, QPointF fromScenePoint, QPointF toScenePoint);

    GraphicsScene *scene;
    GraphicsView *view;
    QPoint indicatorPos;
    QGraphicsItem *indicatorItem;
    QGraphicsItem *indicatorHead;
    Indicator *indicator;
    QGraphicsItemAnimation *animation;

    static bool snapToGrid;
    bool getFollowTime() const;
    void setFollowTime(bool newFollowTime);

    void setBpm(int newBpm);
    int getBpm() const;
    void setPaused(bool newPaused);
    void setZoom(double newZoom);
    double getZoom() const;
    void buildEventList();
    std::vector<Track *> getTracks() const;
    float barWidth();
    void setTimeImpl(double time);

    virtual void keyPressEvent(QKeyEvent * event) override;

    static const int barResolution = 100;



signals:
    void setTime(double time);



protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
private:
    void onZoom(QGraphicsSceneWheelEvent *event);
    void drawGrid(float xOffset = 100.0f);

    void checkTime();

    int bpm;
    int frame, bars;
    int minFrame, maxFrame;
    double zoom;
    float trackTime;
    bool followTime, paused;

    QGraphicsItemGroup *grid;
    std::vector<Track *> tracks;

    QTimeLine *timer;

    std::vector<EventModel *> events;
    std::list<EventModel *> activeEvents;
    std::vector<EventModel *>::iterator nextEvent;

    QList<Track *> *selected;
    TrackGroup *selectionGroup;
    TrackGroup *clipboardGroup;
    void clearSelection();
    void clearClipboard();

};

#endif // TIMELINE_H
