#include "presetbutton.h"

#include <QApplication>
#include <QTimer>
#include <QDebug>
#include <qevent.h>

PresetButton::PresetButton(PresetModel *model, QWidget *parent) : QLabel(parent), title(model->getName().c_str()), leftTimer(new QTimer(this)), rightTimer(new QTimer(this)), model(model)
{
    setText(title);
    leftTimer->setSingleShot(true);
    leftTimer->setInterval(500);
    rightTimer->setSingleShot(true);
    rightTimer->setInterval(500);

    setWordWrap(true);

    setActive(false);

    connect(leftTimer, &QTimer::timeout, this, &PresetButton::onLeftLongPress);
    connect(rightTimer, &QTimer::timeout, this, &PresetButton::onRightLongPress);
}

void PresetButton::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        leftTimer->start();
    }else if (event->button() == Qt::RightButton) {
        rightTimer->start();
    }
}

void PresetButton::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (leftTimer->isActive()) {
            emit releasedInstantly();
        }

        leftTimer->stop();
    }else if (event->button() == Qt::RightButton) {
        rightTimer->stop();
    }
}

/*
void PresetButton::resizeEvent(QResizeEvent* event)
{
    QLabel::resizeEvent(event);

    QFont font = this->font();
    QRect cRect = this->contentsRect();

    if( this->text().isEmpty() )
        return;

    int fontSize = 1;

    while( true )
    {
        QFont f(font);
        f.setPixelSize( fontSize );
        QRect r = QFontMetrics(f).boundingRect( this->text() );
        if (r.height() <= cRect.height() && r.width() <= cRect.width() )
            fontSize++;
        else
            break;
    }

    font.setPixelSize(fontSize);
    this->setFont(font);
}
*/

PresetButton::~PresetButton() {
    delete leftTimer;
    delete rightTimer;
}

void PresetButton::onLeftLongPress() {
    emit leftLongPressed();
}

void PresetButton::onRightLongPress() {
    emit rightLongPressed();
}

bool PresetButton::getActive() const
{
    return active;
}

void PresetButton::setActive(bool newActive)
{
    active = newActive;
    if (active) {
        setStyleSheet("padding :5px; background-color: red");
    } else {
        setStyleSheet("padding :5px; background-color: gray");
    }
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

