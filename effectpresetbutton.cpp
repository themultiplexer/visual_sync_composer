#include "effectpresetbutton.h"

#include <QApplication>
#include <QPushButton>
#include <QTimer>
#include <QDebug>

EffectPresetButton::EffectPresetButton(QString title, EffectPresetModel *model, QWidget *parent)
    : QPushButton(title, parent), m_timer(new QTimer(this)), model(model)
{
    // Timer to detect long press
    m_timer->setSingleShot(true);  // Only trigger once
    m_timer->setInterval(500);    // Set long press duration (1000 ms = 1 second)

    // Connect timer timeout to longPress signal
    connect(m_timer, &QTimer::timeout, this, &EffectPresetButton::onLongPress);

    // Connect button's press and release events to handle long press logic
    connect(this, &QPushButton::pressed, this, &EffectPresetButton::onPressed);
    connect(this, &QPushButton::released, this, &EffectPresetButton::onReleased);
}

EffectPresetButton::~EffectPresetButton() {
    delete m_timer;
}

void EffectPresetButton::onPressed() {
    m_timer->start();
}

void EffectPresetButton::onReleased() {

    if (m_timer->isActive()) {
        emit releasedInstantly();
    }

    m_timer->stop();
}

void EffectPresetButton::onLongPress() {
    qDebug() << "Long press detected!";
    emit longPressed();
}

EffectPresetModel *EffectPresetButton::getModel() const
{
    return model;
}

void EffectPresetButton::setModel(EffectPresetModel *newModel)
{
    model = newModel;
}

