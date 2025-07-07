#include <QWidget>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QApplication>
#include "helper.h"
#include "tubewidget.h"

#ifndef VSCTUBE_H
#define VSCTUBE_H

class VSCTube : public QWidget
{
    Q_OBJECT

public:
signals:
    void valueChanged();
    void buttonPressed(bool right);
    void flashClicked();

public:
    VSCTube(std::string mac, QWidget *parent);
    int value() const;
    void setValue(int val);
    void setPeaked(rgb color);
    void updateGL();
    std::string getMac() const;

private:
    std::string mac;
    QLabel *label;
    QLabel *rightLabel;
    QPushButton *leftButton;
    QPushButton *rightButton;
    QSpinBox *delaySpinBox, *groupSpinBox;
    void onMinusClicked();
    void onPlusClicked();
    void onValueChanged(int);
    TubeWidget *ftube;
    QPushButton *flashButton;
    void onFlashClicked();
};

#endif // VSCTUBE_H
