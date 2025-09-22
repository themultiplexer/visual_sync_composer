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
    PresetModel *model;
    QTimer *leftTimer, *rightTimer;
    bool active;
};

#endif // EFFECTPRESETBUTTON_H
