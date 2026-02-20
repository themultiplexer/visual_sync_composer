#include "controllerabstractor.h"
#include "audiowindow.h"
#include "helper.h"
#include "wifieventprocessor.h"

#include <QColor>

ControllerAbstractor::ControllerAbstractor(AudioWindow *window) : shiftMode(false), colorMode(false), syncMode(false), captureToggle(true) {
    rng = new std::mt19937(dev());
    byteRandom = new std::uniform_int_distribution<std::mt19937::result_type>(0, 255);

    this->audiowindow = window;

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
    audiowindow->tabWidget->setCurrentIndex((page - 1) % 4);
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
    std::vector<VSCSlider *> sliders = { audiowindow->brightnessSlider, audiowindow->speedSlider, audiowindow->effect1Slider, audiowindow->effect2Slider};

    if (sliders[index]->getIsInverted()) {
        sliders[index]->setValue(255 - value);
    } else {
        sliders[index]->setValue(value);
    }

    audiowindow->sliderDidChanged = true;

}

void ControllerAbstractor::onKnobChanged(int index, int value) {
    audiowindow->knobWidgets[index]->setOuterPercentage((float)value/225.0f);
    audiowindow->knobChanged = true;
}

void ControllerAbstractor::onButtonPress(int index) {
    if (index < 4) {
        flashStopButton(index);
        audiowindow->peakEvent(index % 2 + 1);
    } else {
        int specialIndex = index - 4;
        if (specialIndex == 0) {
            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j < 4; ++j) {
                    if (j == 0 || j == 3) {
                        controller->setMatrixButton(i, j, LEDColor::red, brightness);
                    } else {
                        controller->setMatrixButton(i, j, LEDColor::black, brightness);
                    }
                }
            }
            controller->setButton(LEDButton::SHIFT, brightness);
            shiftMode = true;
        } else if (specialIndex == 1)  {
            controller->setButton(LEDButton::REVERSE, brightness);
            audiowindow->ledModifierCheckboxes[2]->setChecked(true);
        } else if (specialIndex == 2)  {
            controller->setButton(LEDButton::TYPE, brightness);
            colorMode = true;
            drawColors();
        } else if (specialIndex == 4)  {
            std::cout << "Button Browse" << std::endl;
            controller->setButton(LEDButton::BROWSE, brightness);
            std::vector<VSCSlider *> sliders = { audiowindow->speedSlider,
                                                audiowindow->effect1Slider,
                                                audiowindow->effect2Slider,
                                                audiowindow->effect3Slider,
                                                audiowindow->effect4Slider };
            for (VSCSlider* slider: sliders) {
                slider->setValue((*byteRandom)(*rng));
            }

            CONFIG_DATA d = audiowindow->slidersToConfig(audiowindow->ep->getMasterconfig());
            audiowindow->ep->setMasterconfig(d);
            audiowindow->ep->sendConfig();
        } else if (specialIndex == 6)  {
            controller->setButton(LEDButton::SYNC, brightness);
            syncMode = true;
            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j < 4; ++j) {
                    LEDColor col = (j>=2) == 0 ? LEDColor::blue : LEDColor::white;
                    controller->setMatrixButton(i, j, col, brightness);
                }
            }
        } else if (specialIndex == 7)  {
            std::cout << "Button Quant" << std::endl;
            controller->setButton(LEDButton::QUANT, brightness);
            audiowindow->ledModifierCheckboxes[4]->setChecked(true);
        } else if (specialIndex == 8)  {
            std::cout << "Button Capture" << std::endl;
            captureToggle = !captureToggle;
            if (captureToggle) {
                controller->setButton(LEDButton::CAPTURE, brightness);
            } else {
                controller->setButton(LEDButton::CAPTURE, 0.0);
            }
        }
    }
}

void ControllerAbstractor::onButtonRelease(int index) {
    if (index < 4) {

    } else {
        int specialIndex = index - 4;
        if (specialIndex == 0) {
            controller->setButton(LEDButton::SHIFT, 0.0);
            drawMatrixOnController();
            shiftMode = false;
        } else if (specialIndex == 1)  {
            controller->setButton(LEDButton::REVERSE, 0.0);
            audiowindow->ledModifierCheckboxes[2]->setChecked(false);
        } else if (specialIndex == 2)  {
            controller->setButton(LEDButton::TYPE, 0.0);
            drawMatrixOnController();
            colorMode = false;
        }  else if (specialIndex == 4)  {
            controller->setButton(LEDButton::BROWSE, brightness / 2.0);
        } else if (specialIndex == 6)  {
            controller->setButton(LEDButton::SYNC, 0.0);
            syncMode = false;
            drawMatrixOnController();
        } else if (specialIndex == 7)  {
            controller->setButton(LEDButton::QUANT, 0.0);
            audiowindow->ledModifierCheckboxes[4]->setChecked(false);
        }
    }
}

void ControllerAbstractor::onMatrixButtonPress(int col, int row) {
    int button = (col * 4) + row;
    std::cout << "Button Pressed" << button << std::endl;
    if (button < 16) {
        if (shiftMode) {
            int index = (col + (row/3*4) );
            audiowindow->peakEvent(0, index + 3); // TODO device handling
        } else if (colorMode) {
            audiowindow->currentColor = audiowindow->buttonColors[col][row];
            audiowindow->peakEvent(0, -1, false);
        } else if (syncMode) {
            audiowindow->changeCoordination(button);
        } else {
            audiowindow->setNewEffect(audiowindow->currentTab * 16 + button);
        }
        controller->setMatrixButton(col, row, LEDColor::white);
    } else {

    }
}
void ControllerAbstractor::onMatrixButtonRelease(int col, int row) {
    int button = (col * 4) + row;
    std::cout << "Button Released" << button << std::endl;
    if (button < 16) {
        if (shiftMode) {
            controller->setMatrixButton(col, row, LEDColor::red);
        } else if (colorMode || syncMode) {

        } else {
            drawMatrixOnController();
            QColor color = audiowindow->effectPresets[audiowindow->currentTab * 16 + button]->getColor();
            setMatrixButton(col, row, color, brightness);
        }
    } else {

    }
}

void ControllerAbstractor::drawMatrixOnController() {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            QColor color = audiowindow->effectPresets[audiowindow->currentTab * 16 + (i*4+j)]->getColor();
            setMatrixButton(i, j, color, brightness / 2.0);
        }
    }
}

void ControllerAbstractor::drawColors()
{
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            std::array<float, 2> col = {((i*4 + j) / 15.0f), (i*4 + j) == 15 ? 0.0 : 1.0};
            audiowindow->buttonColors[i][j] = col;
            setMatrixButton(i, j, col, brightness);
        }
    }
}
