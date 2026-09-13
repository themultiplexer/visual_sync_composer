#include "ogltest.h"
#include "ui/oglwidget.h"

#include <qdebug.h>
#include <QCursor>
#include <qevent.h>

OGLTest::OGLTest(QQuickItem *parent) : QQuickFramebufferObject(parent) {
    setAcceptedMouseButtons(Qt::AllButtons);
    oglw = new OGLWidget();
    setAcceptHoverEvents(true);
    setKeepMouseGrab(true);
    installEventFilter(this);
    setMirrorVertically(true);
}

QQuickFramebufferObject::Renderer * OGLTest::createRenderer() const {
    
    qDebug() << "createRenderer" << this;
    oglw->initializeGL();
    return oglw;
}

void OGLTest::test() {
    qDebug() << "Hi";
}

void OGLTest::mousePressEvent(QMouseEvent *event)
{
    event->accept();
    setKeepMouseGrab(true);
}

bool OGLTest::eventFilter(QObject *obj, QEvent *event) {
    bool got = false;
    float x,y;
    int activeRegionIndex = 0;

    auto hoverEvent = dynamic_cast<QHoverEvent *>(event);
    if (hoverEvent != nullptr) {
        x = ((float)hoverEvent->position().x() / (float)width());
        y = ((float)hoverEvent->position().y() / (float)height());
        got = true;
    }

    auto mouseEvent = dynamic_cast<QMouseEvent *>(event);
    if (mouseEvent != nullptr) {
        x = ((float)mouseEvent->pos().x() / (float)width());
        y = ((float)mouseEvent->pos().y() / (float)height());
        got = true;
        activeRegionIndex = (mouseEvent->modifiers() == Qt::ControlModifier) ? 1 : 0;
    }

    if(!got || !oglw) {
        return false;
    }

    GLFrequencyRegion* active = oglw->getRegions()[activeRegionIndex];
    for (auto& reg : oglw->getRegions()) {
        reg->mouseEvent(x, y);

        if (reg->getNewInside()) {
            setCursor(Qt::OpenHandCursor);
            if (reg->getNewOnLine()) {
                if (!reg->getDragging()) {
                    setCursor(Qt::SizeVerCursor);
                }
            } else if (reg->getNewOnStart() || reg->getNewOnEnd()) {
                setCursor(Qt::SizeHorCursor);
            } else {
                setCursor(Qt::ArrowCursor);
            }
            active = reg;
            
            if (event->type() != QEvent::MouseButtonPress && event->type() != QEvent::MouseButtonRelease) {
                oglw->updateGL(this);
                return true;
            }
        } else {
            setCursor(Qt::ArrowCursor);
        }
    }
    
    if (mouseEvent) {
        if (mouseEvent->button() == Qt::LeftButton) {
            if (event->type() == QEvent::MouseButtonPress) {
                active->mouseClick(x, y);
            } else if (event->type() == QEvent::MouseButtonRelease) {
                active->mouseReleased(x, y);
            }
        }
    }

    oglw->updateGL(this);
    return false;
}