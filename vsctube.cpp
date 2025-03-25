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

void VSCTube::onValueChanged(int s)
{
    emit valueChanged();
}


VSCTube::VSCTube(QString name, QWidget *parent) : QWidget(parent) {
    textEdit = new QSpinBox();
    textEdit->setValue(0);
    textEdit->setMaximumHeight(50);

    leftButton = new QPushButton("<");
    leftButton->setMinimumWidth(20);

    name.resize(12, ' ');

    ftube = new TubeWidget(this);
    ftube->setMinimumHeight(150);
    ftube->setMaximumHeight(150);
    ftube->setMinimumWidth(25);
    ftube->setMaximumWidth(25);

    leftLabel = new QLabel(name);
    rightButton = new QPushButton(">");
    rightButton->setMinimumWidth(20);
    // Create a layout for this widget
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(leftLabel);
    layout->addWidget(ftube);
    layout->addWidget(textEdit);
    layout->addWidget(leftButton);
    layout->addWidget(rightButton);
    setLayout(layout);

    connect(leftButton, &QPushButton::pressed, this, &VSCTube::onMinusClicked);
    connect(rightButton, &QPushButton::pressed, this, &VSCTube::onPlusClicked);
    connect(textEdit, QOverload<int>::of(&QSpinBox::valueChanged), this, &VSCTube::onValueChanged);
}
