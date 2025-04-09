#include "vscslider.h"
#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QApplication>
#include <QFontDatabase>

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
    slider->blockSignals(true);
    rightLabel->setText(QString("%1").arg(slider->value()));
    slider->setValue(val);
    slider->blockSignals(false);
}

VSCSlider::VSCSlider(QString name, Qt::Orientation orientation, QWidget *parent) : QWidget(parent) {
    slider = new QSlider(orientation);

    leftButton = new QPushButton("-");

    name.resize(12, ' ');

    leftLabel = new QLabel(name);
    //leftLabel->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    leftLabel->setMinimumWidth(150);
    rightLabel = new QLabel("Right");
    rightButton = new QPushButton("+");

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

void VSCSlider::onSliderValueChanged(int value)
{
    rightLabel->setText(QString("%1").arg(slider->value()));
    emit valueChanged();
}

void VSCSlider::onMinusClicked()
{
    slider->setValue(slider->value() - 1);
    emit sliderReleased();
}

void VSCSlider::onPlusClicked()
{
    slider->setValue(slider->value() + 1);
    emit sliderReleased();
}

float VSCSlider::pct(){
    return slider->value() / 100.0;
}

void VSCSlider::onSliderReleased()
{
    rightLabel->setText(QString("%1").arg(slider->value()));
    emit sliderReleased();
}

