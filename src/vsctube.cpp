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

int VSCTube::value() const
{
    return delaySpinBox->value();
}

void VSCTube::setValue(int val)
{
    delaySpinBox->setValue(val);
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


void VSCTube::onValueChanged(int s)
{
    emit valueChanged();
}


VSCTube::VSCTube(QString name, QWidget *parent) : QWidget(parent) {
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
    ftube->setMaximumHeight(150);
    ftube->setMinimumWidth(100);
    ftube->setMaximumWidth(100);
    label = new QLabel(name);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(leftButton);
    buttonLayout->addWidget(rightButton);
    buttonLayout->setAlignment(Qt::AlignHCenter);

    flashButton = new QPushButton("Flash");
    flashButton->setMinimumWidth(100);
    flashButton->setMaximumWidth(100);

    // Create a layout for this widget
    QVBoxLayout *outerLayout = new QVBoxLayout;
    outerLayout->addWidget(label);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(ftube);
    QVBoxLayout *controlsLayout = new QVBoxLayout;

    controlsLayout->addLayout(buttonLayout);
    controlsLayout->addWidget(delaySpinBox);
    controlsLayout->addWidget(groupSpinBox);
    controlsLayout->addWidget(flashButton);
    layout->addLayout(controlsLayout);
    outerLayout->addLayout(layout);
    setLayout(outerLayout);

    connect(leftButton, &QPushButton::released, this, &VSCTube::onMinusClicked);
    connect(rightButton, &QPushButton::released, this, &VSCTube::onPlusClicked);
    connect(flashButton, &QPushButton::released, this, &VSCTube::onFlashClicked);
    connect(delaySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &VSCTube::onValueChanged);
    //connect(delaySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &VSCTube::onValueChanged);
}

void VSCTube::setPeaked(rgb color) {
    QTimer::singleShot(delaySpinBox->value(), ftube, [=]() {
        ftube->setPeaked(QColor(color.r, color.g, color.b));  // Request an update every 16ms (~60 FPS)
    });
}

void VSCTube::updateGL() {
    ftube->update();
}
