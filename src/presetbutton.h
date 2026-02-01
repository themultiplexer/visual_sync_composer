#ifndef EFFECTPRESETBUTTON_H
#define EFFECTPRESETBUTTON_H

#include "presetmodel.h"
#include <QLabel>
#include <QTimer>

class PresetButton : public QLabel
{
    Q_OBJECT

public:
    explicit PresetButton(PresetModel *model, QWidget *parent = nullptr);
    ~PresetButton();

    PresetModel *getModel() const;
    void setModel(PresetModel *newModel);

    bool getActive() const;
    void setActive(bool newActive);

    void setColor(QColor color, bool border) {
        bool light = (color.red() + color.green() + color.blue()) > 300;
        QString style("background-color: rgb(%1,%2,%3); border: %4; color: %5;");
        style = style.arg(color.red()).arg(color.green()).arg(color.blue()).arg(border ? "5px solid white" : "5px solid black", light ? "black" : "white");
        this->setStyleSheet(style);
        this->color = color;

    }

signals:
    void releasedInstantly();
    void leftLongPressed();
    void rightLongPressed();

protected:
    void mouseReleaseEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);

    //void resizeEvent(QResizeEvent *event);
private slots:
    void onLeftLongPress();
    void onRightLongPress();

private:
    QString title;
    QColor color;
    PresetModel *model;
    QTimer *leftTimer, *rightTimer;
    bool active;
};

#endif // EFFECTPRESETBUTTON_H
