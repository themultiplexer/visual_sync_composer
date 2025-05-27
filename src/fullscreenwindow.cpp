#include "fullscreenwindow.h"

FullscreenWindow::FullscreenWindow(QWidget *parent) {

}

void FullscreenWindow::keyReleaseEvent(QKeyEvent *event) {
    qDebug() << "Key released:" << event->key();

    if (event->key() == Qt::Key_Escape) {
        emit escapePressed();
    }
}
