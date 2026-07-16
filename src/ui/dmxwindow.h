#ifndef DMXWINDOW_H
#define DMXWINDOW_H

#include <qwidget.h>
#include <vector>
#include "ui/audiowindow.h"
#include "vscslider.h"
#include "core/wifieventprocessor.h"

class AudioWindow;

class DmxWindow : public QWidget
{
    Q_OBJECT
public:
    DmxWindow(AudioWindow *aw, WifiEventProcessor *ep);
private slots:
    void sliderChanged();
private:
    std::vector<VSCSlider *> sliders;
    WifiEventProcessor *ep;
    AudioWindow *aw;
};

#endif // DMXWINDOW_H
