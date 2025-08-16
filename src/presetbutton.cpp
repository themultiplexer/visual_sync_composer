#include "presetbutton.h"

#include <QApplication>
#include <QTimer>
#include <QDebug>

PresetButton::PresetButton(PresetModel *model, QWidget *parent) : QLabel(parent), title(model->getName().c_str()), m_timer(new QTimer(this)), model(model)
{
    setText(title);
    m_timer->setSingleShot(true);
    m_timer->setInterval(500);

    setWordWrap(true);

    setActive(false);

    connect(m_timer, &QTimer::timeout, this, &PresetButton::onLongPress);
}

void PresetButton::mousePressEvent(QMouseEvent* event) {
    m_timer->start();
}

void PresetButton::mouseReleaseEvent(QMouseEvent* event) {
    if (m_timer->isActive()) {
        emit releasedInstantly();
    }

    m_timer->stop();
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
    delete m_timer;
}

void PresetButton::onLongPress() {
    qDebug() << "Long press detected!";
    emit longPressed();
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

