#ifndef AUDIOWINDOW_H
#define AUDIOWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QCloseEvent>
#include <QSplitter>
#include <QOpenGLWidget>
#include <QMessageBox>
#include <QLabel>
#include <QSlider>
#include <QInputDialog>
#include <QLineEdit>
#include "audiofilter.h"
#include "presetbutton.h"
#include "thread.h"
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
#include "ui_mainwindow.h"
#include "vscslider.h"
#include "thread.h"
#include "audioanalyzer.h"
#include "oglwidget.h"
#include "tubewidget.h"
#include "wifieventprocessor.h"
#include <sdbus-c++/sdbus-c++.h>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>

#include "ProjectModel.h"
#include "effectpresetmodel.h"
#include "gltext.h"
#include "wifieventprocessor.h"
#include <boost/circular_buffer.hpp>
#include <QRadioButton>
#include "netdevice.h"
#include "helper.h"
#include "vsctube.h"
#include "tubepresetmodel.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE


enum ColorControl {
    RandomHue,
    RandomColor,
    Palette
};

enum GroupSelection {
    CountUp,
    Bounce,
    Random
};

class AudioWindow : public QMainWindow
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
    const EffectPresetModel *getCurrentEffect() const;

protected:
    void checkTime();
    void modifierChanged(bool state);

private:
    Ui::MainWindow *ui;
    WorkerThread *workerThread;
    WifiEventProcessor *ep;
    VSCSlider *sensitivitySlider, *brightnessSlider, *speedSlider, *effect1Slider, *effect2Slider;
    QLabel *bpmLabel, *tmpLabel;
    QCheckBox *audioCheckBox;
    AudioAnalyzer* a;
    QDockWidget *dock;
    OGLWidget *glv, *popoutGlv;
    const EffectPresetModel* currentEffect;
    const TubePresetModel* currentPreset;
    ColorControl colorMode;
    QLabel *numBeatLabel, *wifiLabel;
    bool lastColorRed;
    int numBeats, numGroups;

    std::string values[16] = {"Hue", "Pump","Tube", "Pump Limiter","Duck","FadeToColor","Sparkle","Fire","Bounce", "Colorcycle", "11", "Strobe", "Random Flicker", "Tunnel", "Tunnel2", "Placeholder"};

    std::vector<QCheckBox*> ledModifierCheckboxes;
    std::vector<QRadioButton*> ledModeRadioButtons;
    std::vector<EffectPresetModel *> effectPresets;
    std::vector<TubePresetModel *> tubePresets;
    std::map<const EffectPresetModel *, PresetButton *> effectButtons{};
    std::map<const TubePresetModel *, PresetButton *> tubeButtons{};

    FixedQueue<uint64_t, 10> beats;
    FixedQueue<std::array<float, 2>, 4> colors;

    std::random_device dev;
    std::mt19937 *rng;
    std::uniform_int_distribution<std::mt19937::result_type> *hueRandom;


    std::uniform_int_distribution<std::mt19937::result_type> *effectRandom;

    CONFIG_DATA currentEffectConfig;

    void closeEvent(QCloseEvent *event);
    void spinboxChanged(int bpm);

    VSCSlider *saturationSlider;
    VSCSlider *effect3Slider;
    VSCSlider *effect4Slider;
    TubeWidget *ftube;
    std::vector<VSCTube*> tubes;
    VSCSlider *timeSlider;
    VSCSlider *otherSlider;

    FullscreenWindow *fullScreenWindow;
    void checkStatus();
    void sendTubeSyncData();
    void applyTubePreset(const TubePresetModel *model);
};
#endif // AUDIOWINDOW_H
