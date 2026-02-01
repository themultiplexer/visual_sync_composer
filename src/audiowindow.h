#ifndef AUDIOWINDOW_H
#define AUDIOWINDOW_H

#include <QMainWindow>
#include "qmainwindow.h"
#include <QWidget>
#include <QCloseEvent>
#include <QSplitter>
#include <QMessageBox>
#include <QLabel>
#include <QSlider>
#include <QInputDialog>
#include <QLineEdit>
#include <QColorDialog>
#include <QGroupBox>
#include "audiofilter.h"
#include "knobwidget.h"
#include "midireceiver.h"
#include "presetbutton.h"
#include <QGuiApplication>
#include <QApplication>
#include <QtNetwork/QNetworkReply>
#include <QThread>
#include <QGraphicsScene>
#include <QSpinBox>
#include <QGraphicsView>
#include <QGuiApplication>
#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QImage>
#include <QGraphicsPixmapItem>
#include <QMenu>
#include <QAction>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QImage>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QThread>
#include <QCheckBox>
#include "fullscreenwindow.h"
#include "vscslider.h"
#include "audioanalyzer.h"
#include "oglwidget.h"
#include "tubewidget.h"
#include "wifieventprocessor.h"
#include <sdbus-c++/sdbus-c++.h>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>

#include "projectmodel.h"
#include "effectpresetmodel.h"
#include "wifieventprocessor.h"
#include <boost/circular_buffer.hpp>
#include <QRadioButton>
#include "helper.h"
#include "vsctube.h"
#include "netdevice.h"
#include "tubepresetmodel.h"
#include "f1_driver/include/controller_handler.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE


enum class ColorControl {
    RandomHue,
    RandomColor,
    Palette,
    Manual,
    Frequency
};

enum class ColorSelectionMode {
    CountUp,
    Regions,
    Random
};

enum class GroupSelection {
    CountUp,
    Regions,
    Random
};

class AudioWindow : public QMainWindow, ControllerDelegate
{
    Q_OBJECT

public:
    AudioWindow(WifiEventProcessor *ep, QWidget *parent = nullptr);
    ~AudioWindow();

    void sliderChanged();
    void effectChanged(bool state);
    void setAlbumArtwork(QPixmap *);
    void bpmChanged(int bpm);
    void loadLyrics();
    void setNewEffect(EffectPresetModel *model);
    int activeEffect;
    EffectPresetModel *getCurrentEffect();
    void setNewEffect(int index);

protected:
    void checkTime();
    void modifierChanged(bool state);
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::MainWindow *ui;
    WifiEventProcessor *ep;
    VSCSlider *sensitivitySlider, *brightnessSlider, *speedSlider, *effect1Slider, *effect2Slider, *effect3Slider, *effect4Slider;
    QTabWidget *tabWidget;
    QLabel *bpmLabel, *tmpLabel;
    QCheckBox *audioCheckBox;
    AudioAnalyzer* a;
    QDockWidget *dock;
    OGLWidget *glv, *popoutGlv;
    int currentEffect, currentTab, currentPreset, currentPaletteIndex, numBeats, numGroups;
    ColorControl colorMode;
    QLabel *numBeatLabel, *wifiLabel;
    bool lastColorRed;
    GroupSelection groupMode;
    ColorSelectionMode colorSelectionMode;
    PresetButton *activeEffectPresetButton;
    PresetButton *activeTubePresetButton;
    std::chrono::time_point<std::chrono::system_clock> lastEffectChange, lastPresetChange, lastDmxSent, lastSliderChange, lastKnobChange;

    ControllerHandler *controller;

    std::string values[20] = {"Hue", "Pump", "Tube", "Pump Limiter","Duck","FadeToColor","Sparkle","Fire","Bounce", "Colorcycle", "11", "Strobe", "Random Flicker", "Tunnel", "Tunnel2", "Placeholder","confetti", "sinelon", "bpm", "juggle"};

    std::vector<QCheckBox*> ledModifierCheckboxes;
    std::vector<QCheckBox*> autoCheckboxes;
    std::vector<QRadioButton*> ledModeRadioButtons;
    std::vector<EffectPresetModel *> effectPresets;
    std::vector<TubePresetModel *> tubePresets;
    std::vector<PresetButton *> effectButtons{};
    std::vector<PresetButton *> tubeButtons{};
    std::vector<KnobWidget *> knobWidgets;

    bool sliderDidChanged;


    FixedQueue<uint64_t, 10> beats;
    FixedQueue<std::array<float, 2>, 1> colors;

    std::array<float, 2> colorPalette;

    std::array<float, 4> buttonAfterglow;

    std::random_device dev;
    std::mt19937 *rng;
    std::uniform_int_distribution<std::mt19937::result_type> *hueRandom, *byteRandom;
    std::uniform_int_distribution<std::mt19937::result_type> *effectRandom;
    std::uniform_int_distribution<std::mt19937::result_type> *presetRandom;
    std::uniform_int_distribution<std::mt19937::result_type> *paletteRandom;

    std::array<std::array<float, 2>, 6> currentPalette;

    CONFIG_DATA currentEffectConfig;

    void closeEvent(QCloseEvent *event) override;
    void spinboxChanged(int bpm);

    VSCSlider *saturationSlider;
    TubeWidget *ftube;
    std::vector<VSCTube*> tubes;
    VSCSlider *timeSlider;
    VSCSlider *otherSlider;
    std::array<float,2> currentColor;
    int lastButton;
    MidiReceiver *receiver;

    std::array<std::array<std::array<float, 2>, 4>, 4> buttonColors;

    QTimer *timer;
    int tubeFrames;

    FullscreenWindow *fullScreenWindow;
    void checkStatus();
    void sendTubeSyncData();
    void applyTubePreset(const TubePresetModel *model);
    CONFIG_DATA slidersToConfig(CONFIG_DATA d);
    void peakEvent(int group = 0, int index = -1, bool pickColor = true);
    bool knobChanged;
    bool shiftPressed;

    void sendToMidiController(int index);
    void drawMatrixOnController();
    
    void sendSliderChanged(int index, int value) override;
    void sendKnobChanged(int index, int value) override;
    void sendButtonPress(int index) override;
    void sendButtonRelease(int index) override;
    void sendWheelChanged(int page) override;
    void sendMatrixButtonPress(int, int) override;
    void sendMatrixButtonRelease(int, int) override;
    void changeCoordination(int index);
};
#endif // AUDIOWINDOW_H
