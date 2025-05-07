#include "horizontal_line.h"

HorizontalLine::HorizontalLine(QWidget* parent)
    : QFrame(parent) {
    setFrameShape(QFrame::HLine);     // Set shape to horizontal line
    setFrameShadow(QFrame::Sunken);  // Optional: make it look recessed
}
