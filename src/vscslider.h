#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QApplication>

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
    VSCSlider(QString name, Qt::Orientation orientation, QWidget *parent);

    int minimum() const;
    void setMinimum(int min);

    int maximum() const;
    void setMaximum(int max);
    int value() const;
    void setValue(int val);
    float pct();
private slots:
    void onSliderReleased();
    void onSliderValueChanged(int value);
    void onMinusClicked();
    void onPlusClicked();
private:
    QSlider *slider;
    QLabel *leftLabel;
    QLabel *rightLabel;
    QPushButton *leftButton;
    QPushButton *rightButton;
};

#endif // VSCSLIDER_H
