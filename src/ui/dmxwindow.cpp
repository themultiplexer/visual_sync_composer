#include "dmxwindow.h"
#include "audiowindow.h"
#include <qboxlayout.h>
#include <qwidget.h>

DmxWindow::DmxWindow(WifiEventProcessor *ep) {
    this->ep = ep;
    QVBoxLayout *superLayout = new QVBoxLayout(this);

    for (int i = 0; i < 10; ++i) {
        VSCSlider *slider = new VSCSlider(("Channel " + std::to_string(i)).c_str(), Qt::Horizontal, this);
        superLayout->addWidget(slider);
        sliders.push_back(slider);
        connect(slider, &VSCSlider::valueChanged, this, &DmxWindow::sliderChanged);
    }

    this->setLayout(superLayout);
}


void DmxWindow::sliderChanged()
{
    ptrdiff_t index = std::distance(sliders.begin(), std::find(sliders.begin(), sliders.end(), sender()));
    std::vector<double> channels(sliders.size());
    for (size_t i = 0; i < sliders.size(); ++i) {
        channels[i] = sliders[i]->pct();
    }
    ep->sendDmx(channels);
}
