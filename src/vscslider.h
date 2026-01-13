#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QApplication>
#include <qcheckbox.h>

#ifndef VSCSLIDER_H
#define VSCSLIDER_H

class VSCSlider : public QWidget
{
    Q_OBJECT

public:
signals:
    // Define a custom signal that emits the slider's value
    void sliderReleased();
    void valueChanged();

public:
    VSCSlider(QString name, Qt::Orientation orientation, QWidget *parent, bool inverted = false);

    int minimum() const;
    void setMinimum(int min);

    int maximum() const;
    void setMaximum(int max);
    int value() const;
    void setValue(int val);
    float pct();
    bool getIsInverted() const;

private slots:
    void onSliderReleased();
    void onSliderValueChanged(int value);
    void onMinusClicked();
    void onPlusClicked();
    void lockToggled(bool checked);

private:
    QSlider *slider;
    QLabel *leftLabel;
    QLabel *rightLabel;
    QPushButton *leftButton;
    QPushButton *rightButton;
    QCheckBox *checkbox;

    bool locked, isInverted;

};

#endif // VSCSLIDER_H
