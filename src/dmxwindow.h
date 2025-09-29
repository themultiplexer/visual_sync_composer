#ifndef DMXWINDOW_H
#define DMXWINDOW_H

#include <qwidget.h>
#include <vector>
#include "vscslider.h"
#include "wifieventprocessor.h"

class DmxWindow : public QWidget
{
    Q_OBJECT
public:
    DmxWindow(WifiEventProcessor *ep);
private slots:
    void sliderChanged();
private:
    std::vector<VSCSlider *> sliders;
    WifiEventProcessor *ep;
};

#endif // DMXWINDOW_H
