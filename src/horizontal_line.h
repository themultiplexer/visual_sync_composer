#ifndef HORIZONTAL_LINE_H
#define HORIZONTAL_LINE_H

#include <QFrame>

class HorizontalLine : public QFrame {
    Q_OBJECT

public:
    explicit HorizontalLine(QWidget* parent = nullptr);
};

#endif // HORIZONTAL_LINE_H
