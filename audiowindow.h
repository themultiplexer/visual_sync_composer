#ifndef AUDIOWINDOW_H
#define AUDIOWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QOpenGLWidget>
#include <QMessageBox>
#include <QLabel>
#include <QSlider>
#include <QInputDialog>
#include <QLineEdit>
#include "effectpresetbutton.h"
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
#include "ui_mainwindow.h"
#include "vscslider.h"
#include "thread.h"
#include "audioanalyzer.h"
#include "oglwidget.h"
#include "wifieventprocessor.h"
#include <sdbus-c++/sdbus-c++.h>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

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

protected:
    void checkTime();
private:
    Ui::MainWindow *ui;
    WorkerThread *workerThread;
    WifiEventProcessor *ep;
    VSCSlider *sensitivitySlider, *brightnessSlider, *speedSlider, *effect1Slider, *effect2Slider;
    AudioAnalyzer* a;
    OGLWidget *glv;
    std::string values[16] = {"Hue", "Pump","Tube", "Pump Limiter","Duck","FadeToColor","Sparkle","Fire","Bounce", "Colorcycle", "11", "Strobe", "Random Flicker", "Tunnel", "Tunnel2", "Placeholder"};

    CONFIG_DATA currentEffectConfig;

    void closeEvent(QCloseEvent *event);
    void spinboxChanged(int bpm);
};
#endif // AUDIOWINDOW_H
