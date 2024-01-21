#ifndef COLORLABEL_H
#define COLORLABEL_H

#include <QLabel>

class ColorLabel : public QLabel
{
public:
    ColorLabel(const QString &text, QWidget *parent = nullptr)
        : QLabel(text, parent)
    {
        setAutoFillBackground(true);
    }

    void setTextColor(const QColor &color)
    {
        QPalette palette = this->palette();
        palette.setColor(this->backgroundRole(), color);
        palette.setColor(this->foregroundRole(), color);
        this->setPalette(palette);
    }
};

#endif // COLORLABEL_H
