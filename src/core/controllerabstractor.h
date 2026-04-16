#ifndef CONTROLLERABSTRACTOR_H
#define CONTROLLERABSTRACTOR_H

#include "f1_driver/include/controller_handler.h"
#include <qcolor.h>
#include <random>
#include <vector>

class AudioWindow;

class ControllerAbstractor : ControllerDelegate
{
public:
    ControllerAbstractor(AudioWindow *window);

    void processEvents();
    bool isCaptureToggled();
    bool captureToggle;
    ControllerHandler *controller;
    std::random_device dev;
    std::mt19937 *rng;
    std::uniform_int_distribution<std::mt19937::result_type> *byteRandom;

    // This is not clean but I am lazy
    AudioWindow *audiowindow;

    std::array<float, 4> buttonAfterglow;

    void onSliderChanged(int index, int value);
    void onKnobChanged(int index, int value);
    void onButtonPress(int index);
    void onButtonRelease(int index);
    void onMatrixButtonPress(int col, int row);
    void onMatrixButtonRelease(int col, int row);

    void flashStopButton(int index);
    void setStopButtons(std::array<float, 4> buttonBrightness);
    void onWheelChanged(int page);
    void setPage(int page);
    void setMatrixButton(int col, int row, QColor color, float brightness = 1.0);
    void setMatrixButton(int col, int row, std::array<float, 2> color, float brightness);
    void updateMatrix();
private:
    void drawMatrixOnController();
    std::vector<int> getPressedButtons();

    bool shiftMode, colorMode, syncMode;
    float brightness;
    std::vector<int> pressedButtons;
    void drawColors();
    void drawGroups();
    void drawSyncModes();
};

#endif // CONTROLLERABSTRACTOR_H
