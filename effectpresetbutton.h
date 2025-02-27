#ifndef EFFECTPRESETBUTTON_H
#define EFFECTPRESETBUTTON_H

#include "effectpresetmodel.h"
#include <QPushButton>
#include <QTimer>

class EffectPresetButton : public QPushButton
{
    Q_OBJECT

public:
    explicit EffectPresetButton(QString title, EffectPresetModel *model, QWidget *parent = nullptr);
    ~EffectPresetButton();


    EffectPresetModel *getModel() const;
    void setModel(EffectPresetModel *newModel);

signals:
    void releasedInstantly();
    void released();
    void longPressed();

private slots:
    void onPressed();
    void onReleased();
    void onLongPress();

private:
    QTimer *m_timer;
    EffectPresetModel *model;
};

#endif // EFFECTPRESETBUTTON_H
