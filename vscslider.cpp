#include "vscslider.h"
#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QApplication>

int VSCSlider::minimum() const
{
    return slider->minimum();
}

void VSCSlider::setMinimum(int min)
{
    slider->setMinimum(min);
}

int VSCSlider::maximum() const
{
    return slider->maximum();
}

void VSCSlider::setMaximum(int max)
{
    slider->setMaximum(max);
}

int VSCSlider::value() const
{
    return slider->value();
}

void VSCSlider::setValue(int val)
{
    slider->setValue(val);
}

VSCSlider::VSCSlider(QString name, Qt::Orientation orientation, QWidget *parent) : QWidget(parent) {
    // Create a slider, a left label and a right label
    slider = new QSlider(orientation);
    leftButton = new QPushButton("-");

    name.resize(15, ' ');

    leftLabel = new QLabel(name);
    rightLabel = new QLabel("Right");
    rightButton = new QPushButton("+");

    // Optionally, set the minimum and maximum values for the slider
    slider->setMinimum(0);
    slider->setMaximum(100);

    // Create a layout for this widget
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(leftLabel);
    layout->addWidget(leftButton);
    layout->addWidget(slider);
    layout->addWidget(rightLabel);
    layout->addWidget(rightButton);
    setLayout(layout);

    connect(leftButton, &QPushButton::pressed, this, &VSCSlider::onMinusClicked);
    connect(rightButton, &QPushButton::pressed, this, &VSCSlider::onPlusClicked);
    connect(slider, &QSlider::sliderReleased, this, &VSCSlider::onSliderReleased);
    connect(slider, &QSlider::valueChanged, this, &VSCSlider::onSliderValueChanged);
}

// Slot to update the labels when the slider value changes
void VSCSlider::onSliderValueChanged(int value)
{
    // You can customize the labels' content based on the slider value
    rightLabel->setText(QString("%1").arg(slider->value())); // For example, show complementary value on right
}

void VSCSlider::onMinusClicked()
{
    slider->setValue(slider->value() - 1);
}

void VSCSlider::onPlusClicked()
{
    slider->setValue(slider->value() + 1);
}

// Slot to update the labels when the slider value changes
void VSCSlider::onSliderReleased()
{
    // You can customize the labels' content based on the slider value
    rightLabel->setText(QString("%1").arg(slider->value())); // For example, show complementary value on right
    emit sliderReleased();
}

