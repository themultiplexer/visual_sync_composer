#include "ogltest.h"
#include "oglrenderer.h"
#include "ui/oglwidget.h"
#include <qdebug.h>

OGLTest::OGLTest(QQuickItem *parent) : QQuickFramebufferObject(parent) {
    setAcceptedMouseButtons(Qt::AllButtons);
}

QQuickFramebufferObject::Renderer * OGLTest::createRenderer() const {
    auto oglw = new OGLWidget();
    oglw->initializeGL();
    return oglw;
}

void OGLTest::test() {
    qDebug() << "Hi";
}