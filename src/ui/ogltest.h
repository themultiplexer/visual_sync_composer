// ogltest.h
#pragma once

#include <QObject>
#include <QtQuick/QQuickFramebufferObject>
#include <QtQml/qqmlregistration.h>

#include "core/frequencyregion.h"
#include "ui/oglwidget.h"

class OGLTest : public QQuickFramebufferObject
{
    Q_OBJECT
public:
    explicit OGLTest(QQuickItem *parent = nullptr);
    QQuickFramebufferObject::Renderer *createRenderer() const override;
    void test();

public slots:

    void setSpectrum(const QVector<float> &data)
    {
        m_spectrum = data;
        oglw->updateGL(this);
    }

    void setRegions(const QVector<FrequencyRegion *> &data)
    {
        qDebug() << "setRegions" << data;
        m_regions = data;
        oglw->updateGL(this);
    }

private:
    QVector<float> m_spectrum;
    QVector<FrequencyRegion *> m_regions;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

    OGLWidget *oglw;

    friend class OGLWidget;
};