// ogltest.h
#pragma once

#include <QObject>
#include <QtQuick/QQuickFramebufferObject>
#include <QtQml/qqmlregistration.h>

class OGLTest : public QQuickFramebufferObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit OGLTest(QQuickItem *parent = nullptr);
    QQuickFramebufferObject::Renderer *createRenderer() const override;
    void test();
signals:
    void sigtest();
};