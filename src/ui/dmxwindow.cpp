#include "dmxwindow.h"
#include "audiowindow.h"
#include <cstdint>
#include <qboxlayout.h>
#include <qwidget.h>

DmxWindow::DmxWindow(AudioWindow *aw, WifiEventProcessor *ep) {
    this->ep = ep;
    this->aw = aw;

    QVBoxLayout *superLayout = new QVBoxLayout(this);

    for (int i = 0; i < 10; ++i) {
        VSCSlider *slider = new VSCSlider(("Channel " + std::to_string(i)).c_str(), Qt::Horizontal, this);
        slider->setMaximum(255);
        superLayout->addWidget(slider);
        sliders.push_back(slider);
        connect(slider, &VSCSlider::valueChanged, this, &DmxWindow::sliderChanged);
    }

    this->setLayout(superLayout);
}


void DmxWindow::sliderChanged()
{
    ptrdiff_t index = std::distance(sliders.begin(), std::find(sliders.begin(), sliders.end(), sender()));
    std::vector<uint8_t> channels(sliders.size());
    for (size_t i = 0; i < sliders.size(); ++i) {
        channels[i] = sliders[i]->pct() * 255;
    }
    aw->setDmxChannels(channels);
    ep->sendDmx(channels);
}
