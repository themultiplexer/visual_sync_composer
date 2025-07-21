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
    void delayChanged();
    void groupChanged();
    void valueChanged();
    void buttonPressed(bool right);
    void flashClicked();

public:
    VSCTube(std::string mac, QWidget *parent);
    void setPeaked(rgb color);
    void updateGL();
    std::string getMac() const;

    int getDelay() const;
    int getGroup() const;
    void setDelay(int val);
    void setGroup(int val);
private:
    std::string mac;
    QLabel *label;
    QLabel *rightLabel;
    QPushButton *leftButton;
    QPushButton *rightButton;
    QSpinBox *delaySpinBox, *groupSpinBox;
    void onMinusClicked();
    void onPlusClicked();
    TubeWidget *ftube;
    QPushButton *flashButton;
    void onFlashClicked();
    void onDelayValueChanged(int s);
    void onGroupValueChanged(int s);
};

#endif // VSCTUBE_H
