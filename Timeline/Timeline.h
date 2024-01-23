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
#include "Track.h"
#include "GraphicsView.h"
#include "TrackGroup.h"
#include "AlignedTextItem.h"

class TimeLine : public QWidget
{
    Q_OBJECT

public:
    explicit TimeLine(QWidget *parent = nullptr);
    ~TimeLine();
    QGraphicsItem* ItemAt(QPointF position){return scene->itemAt(position,QTransform());}
    void AddItem(float start, float length, int lane,  QColor color);
    void SetFrame(int _frame){if(_frame < maxFrame)frame=_frame;}
    void setTrackTime(float time);
    void Clear();
    void startAnimation();
    void timerFinished();
    void updateAnimation();
    void rubberBandChanged(QRect viewportRect, QPointF fromScenePoint, QPointF toScenePoint);
    std::vector<std::tuple<float, float, int>> Serialize();

    GraphicsScene *scene;
    GraphicsView *view;
    QPoint indicatorPos;
    QGraphicsItem *indicatorItem;
    QGraphicsItem *indicatorHead;
    Indicator *indicator;
    QGraphicsItemAnimation *animation;

    bool getFollowTime() const;

    void setFollowTime(bool newFollowTime);
    virtual void keyPressEvent(QKeyEvent * event) override;


signals:
    void setTime(double time);



protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
private:
    void onZoom(QWheelEvent *event);
    void drawGrid(int xOffset = 100);
    void setTimeImpl(double time);
    void checkTime();

    int frame, bars;
    int minFrame, maxFrame;
    float zoom;
    float trackTime;
    bool followTime;
    int barResolution;
    QGraphicsItemGroup *grid;
    std::vector<Track *> tracks;
    QTimeLine *timer;
    QList<Track *> *selected;
    TrackGroup *selectionGroup;
    TrackGroup *clipboardGroup;
    void clearSelection();
    void clearClipboard();
};

#endif // TIMELINE_H
