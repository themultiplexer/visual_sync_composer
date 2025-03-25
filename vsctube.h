#include <QWidget>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QApplication>
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

public:
    VSCTube(QString name, QWidget *parent);
    int value() const;
    void setValue(int val);
private:
    QLabel *leftLabel;
    QLabel *rightLabel;
    QPushButton *leftButton;
    QPushButton *rightButton;
    QSpinBox *textEdit;
    void onMinusClicked();
    void onPlusClicked();
    void onValueChanged(int);
    TubeWidget *ftube;
};

#endif // VSCTUBE_H
