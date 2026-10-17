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
    Q_PROPERTY(bool renderingEnabled
               READ renderingEnabled
               WRITE setRenderingEnabled
               NOTIFY renderingEnabledChanged)

public:
    bool renderingEnabled() const { return renderingEnabled_; }

    void setRenderingEnabled(bool enabled)
    {
        if (renderingEnabled_ == enabled)
            return;

        renderingEnabled_ = enabled;
        emit renderingEnabledChanged();

        if (enabled)
            update();
    }

    explicit OGLTest(QQuickItem *parent = nullptr);
    QQuickFramebufferObject::Renderer *createRenderer() const override;
    void test();

public slots:

    void setSpectrum(const QVector<float> &data)
    {
        m_spectrum = data;
        //update();
    }

    void setRegions(const QVector<FrequencyRegion *> &data)
    {
        m_regions = data;
        //update();
    }

signals:
    void renderingEnabledChanged();

private:
    bool renderingEnabled_ = true;
    QVector<float> m_spectrum;
    QVector<FrequencyRegion *> m_regions;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

    OGLWidget *oglw;

    friend class OGLWidget;
};