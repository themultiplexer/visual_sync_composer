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
#include "Indicator.h"
#include "Track.h"
#include "GraphicsView.h"

class TimeLine : public QWidget
{
    Q_OBJECT

public:
    explicit TimeLine(QWidget *parent = nullptr);
    TimeLine(GraphicsView *_view, QWidget *_parent = nullptr);
    ~TimeLine();
    QGraphicsItem* ItemAt(QPointF position){return scene->itemAt(position,QTransform());}
    void AddItem(float start, float length, int lane,  QColor color);
    void SetFrame(int _frame){if(_frame < maxFrame)frame=_frame;}
    void setTrackTime(float time);
    void Clear();
    void startAnimation();
    void updateAnimation();
    void rubberBandChanged(QRect viewportRect, QPointF fromScenePoint, QPointF toScenePoint);
    std::vector<std::tuple<float, float, int>> Serialize();

    QGraphicsScene *scene;
    GraphicsView *view;
    QPoint indicatorPos;
    QGraphicsItem *indicatorItem;
    QGraphicsItem *indicatorHead;
    Indicator *indicator;
    QGraphicsItemAnimation *animation;

    bool getFollowTime() const;

    void setFollowTime(bool newFollowTime);

signals:
    void setTime(double time);

private:
    virtual void wheelEvent(QWheelEvent *event) override;
    void onZoom(QWheelEvent *event);
    void drawGrid(int xOffset = 100);
    void setTimeImpl(double time);
    void checkTime();

    int frame, bars;
    int minFrame, maxFrame;
    float zoom;
    bool followTime;
    int barResolution;
    QGraphicsItemGroup *grid;
    std::vector<Track *> tracks;
    QTimeLine *timer;
};

#endif // TIMELINE_H
