#include "core/controllerabstractor.h"
#include "helper.h"
#include "wifieventprocessor.h"

#include <QColor>

ControllerAbstractor::ControllerAbstractor(AudioController *window) : shiftMode(false), colorMode(false), syncMode(false), captureToggle(true) {
    rng = new std::mt19937(dev());
    byteRandom = new std::uniform_int_distribution<std::mt19937::result_type>(0, 255);

    this->audiocontroller = window;

    brightness = 0.6f;

    controller = new ControllerHandler();
    controller->setDelegate(this);
    controller->setButton(LEDButton::CAPTURE, brightness);
}

void ControllerAbstractor::processEvents()
{
    // Makes controller in the lib fetch data from hidapi
    controller->run();

    setStopButtons(buttonAfterglow);
    for (int i = 0; i < 4; i++) {
        if (buttonAfterglow[i] > 0.0) {
            buttonAfterglow[i] -= 0.05;
            controller->setStopButton(i, buttonAfterglow[i]);
        }
    }
}

bool ControllerAbstractor::isCaptureToggled()
{
    return captureToggle;
}

void ControllerAbstractor::setStopButtons(std::array<float, 4> buttonBrightness)
{
    for (int i = 0; i < 4; i++) {
        if (buttonBrightness[i] > 0.0) {
            controller->setStopButton(i, buttonBrightness[i]);
        }
        /* // If not afterglow
         * buttonAfterglow[i] = buttonBrightness[i];
         */
    }
}

void ControllerAbstractor::setMatrixButton(int row, int col, std::array<float, 2> color, float brightness)
{
    hsv in = hsv { color[0] * 360.0, color[1] * 1.0, 1.0 };
    rgb a = hsv2rgb(in);
    controller->setMatrixButton(row, col, BRGColor {(uint8_t)(a.b * 255.0), (uint8_t)(a.r * 255.0), (uint8_t)(a.g * 255.)}, brightness);
}

void ControllerAbstractor::flashStopButton(int index) {
    buttonAfterglow[index] = 1.0;
}

void ControllerAbstractor::onWheelChanged(int page) {
    //audiocontroller->tabWidget->setCurrentIndex((page - 1) % 5);
}

void ControllerAbstractor::setPage(int page)
{
    controller->setPage(page);
}

void ControllerAbstractor::setMatrixButton(int col, int row, QColor color, float brightness)
{
    BRGColor bgrcolor = BRGColor {(uint8_t)color.blue(), (uint8_t)color.red(), (uint8_t)color.green()};
    controller->setMatrixButton(col, row, bgrcolor, brightness);
}

void ControllerAbstractor::updateMatrix()
{
    drawMatrixOnController();
}

void ControllerAbstractor::onSliderChanged(int index, int value) {


}

void ControllerAbstractor::onKnobChanged(int index, int value) {

}

void ControllerAbstractor::onButtonPress(int index) {

}

void ControllerAbstractor::onButtonRelease(int index) {

}

void ControllerAbstractor::onMatrixButtonPress(int col, int row) {

}

std::vector<int> ControllerAbstractor::getPressedButtons() {
    return pressedButtons;
}

void ControllerAbstractor::onMatrixButtonRelease(int col, int row) {

}

void ControllerAbstractor::drawMatrixOnController() {

}

void ControllerAbstractor::drawColors()
{
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            std::array<float, 2> col = {((i*4 + j) / 15.0f), (i*4 + j) == 15 ? 0.0f : 1.0f};
            //audiocontroller->buttonColors[i][j] = col;
            setMatrixButton(i, j, col, brightness);
        }
    }
}

void ControllerAbstractor::drawGroups()
{
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (j == 0 || j == 3) {
                controller->setMatrixButton(i, j, LEDColor::red, brightness);
            } else {
                controller->setMatrixButton(i, j, LEDColor::black, brightness);
            }
        }
    }
}

void ControllerAbstractor::drawSyncModes()
{
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            LEDColor col = (j>=2) == 0 ? LEDColor::blue : LEDColor::white;
            controller->setMatrixButton(i, j, col, brightness);
        }
    }
}
