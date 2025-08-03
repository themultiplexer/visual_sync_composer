#include <QWidget>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QApplication>
#include <random>
#include "helper.h"
#include "tubewidget.h"

#ifndef VSCTUBE_H
#define VSCTUBE_H

class VSCTube : public QWidget
{
    Q_OBJECT

public:
signals:
    void delayChanged();
    void groupChanged();
    void valueChanged();
    void buttonPressed(bool right);
    void flashClicked();

public:
    VSCTube(std::string mac, QWidget *parent);
    void setPeaked(rgb color, int group);
    void updateGL();
    std::string getMac() const;

    int getDelay() const;
    int getGroup() const;
    void setDelay(int val);
    void setGroup(int val);
    void setEffect(CONFIG_DATA effect);
    void sync();
private:
    std::string mac;
    QLabel *label;
    QLabel *rightLabel;
    QPushButton *leftButton;
    QPushButton *rightButton;
    TubeWidget *ftube;
    QPushButton *flashButton;
    QSpinBox *delaySpinBox, *groupSpinBox;

    void onMinusClicked();
    void onPlusClicked();

    void onFlashClicked();
    void onDelayValueChanged(int s);
    void onGroupValueChanged(int s);
};

#endif // VSCTUBE_H
