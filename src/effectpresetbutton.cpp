#include "effectpresetbutton.h"

#include <QApplication>
#include <QPushButton>
#include <QTimer>
#include <QDebug>

EffectPresetButton::EffectPresetButton(EffectPresetModel *model, QWidget *parent) : QPushButton(parent), title(model->getName().c_str()), m_timer(new QTimer(this)), model(model)
{
    setText(title);
    m_timer->setSingleShot(true);
    m_timer->setInterval(500);

    connect(m_timer, &QTimer::timeout, this, &EffectPresetButton::onLongPress);
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
    setText(model->getName().c_str());
}

