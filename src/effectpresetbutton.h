#ifndef EFFECTPRESETBUTTON_H
#define EFFECTPRESETBUTTON_H

#include "effectpresetmodel.h"
#include <QPushButton>
#include <QTimer>

class EffectPresetButton : public QPushButton
{
    Q_OBJECT

public:
    explicit EffectPresetButton(EffectPresetModel *model, QWidget *parent = nullptr);
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
    QString title;
    EffectPresetModel *model;
    QTimer *m_timer;
};

#endif // EFFECTPRESETBUTTON_H
