#ifndef FULLSCREENWINDOW_H
#define FULLSCREENWINDOW_H

#include "qdebug.h"
#include "qevent.h"
#include "qobjectdefs.h"
#include "qwidget.h"
#include "qmainwindow.h"

class FullscreenWindow : public QMainWindow {
    Q_OBJECT

public:
    FullscreenWindow(QWidget *parent = nullptr);

signals:
    void escapePressed();

protected:
    void keyReleaseEvent(QKeyEvent *event) override;
};

#endif // FULLSCREENWINDOW_H
