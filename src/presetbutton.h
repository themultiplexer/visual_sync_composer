#ifndef EFFECTPRESETBUTTON_H
#define EFFECTPRESETBUTTON_H

#include "effectpresetmodel.h"
#include "presetmodel.h"
#include <QPushButton>
#include <QTimer>

class PresetButton : public QPushButton
{
    Q_OBJECT

public:
    explicit PresetButton(PresetModel *model, QWidget *parent = nullptr);
    ~PresetButton();


    PresetModel *getModel() const;
    void setModel(PresetModel *newModel);


signals:
    void releasedInstantly();
    void released();
    void longPressed();

private slots:
    void onPressed();
    void onReleased();
    void onLongPress();

private:
    QString title;
    PresetModel *model;
    QTimer *m_timer;
};

#endif // EFFECTPRESETBUTTON_H
