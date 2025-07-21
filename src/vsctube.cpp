#include "vsctube.h"
#include "qtimer.h"
#include "tubewidget.h"
#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QApplication>
#include <QFontDatabase>

int VSCTube::getDelay() const
{
    return delaySpinBox->value();
}

int VSCTube::getGroup() const
{
    return groupSpinBox->value();
}

void VSCTube::setDelay(int val)
{
    delaySpinBox->setValue(val);
}

void VSCTube::setGroup(int val)
{
    groupSpinBox->setValue(val);
}

void VSCTube::onMinusClicked()
{
    emit buttonPressed(false);
}

void VSCTube::onPlusClicked()
{
    emit buttonPressed(true);
}

void VSCTube::onFlashClicked()
{
    emit flashClicked();
}


void VSCTube::onDelayValueChanged(int s)
{
    emit valueChanged();
    emit delayChanged();
}

void VSCTube::onGroupValueChanged(int s)
{
    emit valueChanged();
    emit groupChanged();
}


VSCTube::VSCTube(std::string mac, QWidget *parent) : QWidget(parent) {
    this->mac = mac;
    delaySpinBox = new QSpinBox();
    delaySpinBox->setValue(0);
    delaySpinBox->setMaximumHeight(50);
    delaySpinBox->setMaximum(1000);

    groupSpinBox = new QSpinBox();
    groupSpinBox->setValue(0);
    groupSpinBox->setMaximumHeight(50);
    groupSpinBox->setMaximum(1000);

    leftButton = new QPushButton("<");
    leftButton->setMinimumWidth(20);
    leftButton->setMaximumWidth(50);
    rightButton = new QPushButton(">");
    rightButton->setMinimumWidth(20);
    rightButton->setMaximumWidth(50);

    ftube = new TubeWidget(this);
    ftube->setMinimumHeight(150);
    ftube->setMinimumWidth(25);
    ftube->setMaximumWidth(75);
    label = new QLabel(QString::fromStdString(mac));

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(leftButton);
    buttonLayout->addWidget(rightButton);
    buttonLayout->setAlignment(Qt::AlignHCenter);

    flashButton = new QPushButton("Flash");
    flashButton->setMinimumWidth(100);
    flashButton->setMaximumWidth(100);

    // Create a layout for this widget
    QVBoxLayout *outerLayout = new QVBoxLayout;

    QHBoxLayout *layout = new QHBoxLayout;

    QWidget *controlsWidget = new QWidget;
    QVBoxLayout *controlsLayout = new QVBoxLayout(controlsWidget);

    QHBoxLayout *groupLayout = new QHBoxLayout;
    groupLayout->addWidget(groupSpinBox);
    groupLayout->addWidget(new QLabel("G"));

    QHBoxLayout *delayLayout = new QHBoxLayout;
    delayLayout->addWidget(delaySpinBox);
    delayLayout->addWidget(new QLabel("D"));

    controlsLayout->addLayout(buttonLayout);
    controlsLayout->addLayout(delayLayout);
    controlsLayout->addLayout(groupLayout);
    controlsLayout->addWidget(flashButton);

    layout->addWidget(ftube);
    layout->addWidget(controlsWidget);

    outerLayout->addWidget(label);
    outerLayout->addLayout(layout);
    setLayout(outerLayout);

    connect(leftButton, &QPushButton::released, this, &VSCTube::onMinusClicked);
    connect(rightButton, &QPushButton::released, this, &VSCTube::onPlusClicked);
    connect(flashButton, &QPushButton::released, this, &VSCTube::onFlashClicked);
    connect(delaySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &VSCTube::onDelayValueChanged);
    connect(groupSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &VSCTube::onGroupValueChanged);
}

void VSCTube::setPeaked(rgb color) {
    QTimer::singleShot(delaySpinBox->value(), ftube, [=]() {
        ftube->setPeaked(QColor(color.r * 255, color.g * 255, color.b * 255));  // Request an update every 16ms (~60 FPS)
    });
}

void VSCTube::updateGL() {
    ftube->update();
}

std::string VSCTube::getMac() const
{
    return mac;
}
