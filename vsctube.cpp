#include "vsctube.h"
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
    return textEdit->value();
}

void VSCTube::setValue(int val)
{
    textEdit->setValue(val);
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
    textEdit = new QSpinBox();
    textEdit->setValue(0);
    textEdit->setMaximumHeight(50);
    textEdit->setMaximum(1000);

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
    flashButton->setMinimumWidth(20);
    flashButton->setMaximumWidth(50);

    // Create a layout for this widget
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(ftube);
    layout->addWidget(label);
    layout->addLayout(buttonLayout);
    layout->addWidget(textEdit);
    layout->addWidget(leftButton);
    layout->addWidget(rightButton);
    layout->addWidget(flashButton);
    setLayout(layout);

    connect(leftButton, &QPushButton::released, this, &VSCTube::onMinusClicked);
    connect(rightButton, &QPushButton::released, this, &VSCTube::onPlusClicked);
    connect(flashButton, &QPushButton::released, this, &VSCTube::onFlashClicked);
    connect(textEdit, QOverload<int>::of(&QSpinBox::valueChanged), this, &VSCTube::onValueChanged);
}

void VSCTube::setPeaked(QColor color) {
    ftube->setPeaked(color);
}

void VSCTube::updateGL() {
    ftube->update();
}
