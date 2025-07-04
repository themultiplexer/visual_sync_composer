#include "presetbutton.h"

#include <QApplication>
#include <QPushButton>
#include <QTimer>
#include <QDebug>

PresetButton::PresetButton(PresetModel *model, QWidget *parent) : QPushButton(parent), title(model->getName().c_str()), m_timer(new QTimer(this)), model(model)
{
    setText(title);
    m_timer->setSingleShot(true);
    m_timer->setInterval(500);

    connect(m_timer, &QTimer::timeout, this, &PresetButton::onLongPress);
    connect(this, &QPushButton::pressed, this, &PresetButton::onPressed);
    connect(this, &QPushButton::released, this, &PresetButton::onReleased);
}

PresetButton::~PresetButton() {
    delete m_timer;
}

void PresetButton::onPressed() {
    m_timer->start();
}

void PresetButton::onReleased() {

    if (m_timer->isActive()) {
        emit releasedInstantly();
    }

    m_timer->stop();
}

void PresetButton::onLongPress() {
    qDebug() << "Long press detected!";
    emit longPressed();
}

PresetModel *PresetButton::getModel() const
{
    return model;
}

void PresetButton::setModel(PresetModel *newModel)
{
    model = newModel;
    setText(model->getName().c_str());
}

