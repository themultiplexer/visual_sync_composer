#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QSlider>
#include "downloader.h"
#include "thread.h"
#include "Timeline/GraphicsView.h"
#include "Timeline/Timeline.h"
#include <QGuiApplication>
#include <QApplication>
#include <QtNetwork/QNetworkReply>
#include <QThread>
#include <iostream>
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
#include "./ui_mainwindow.h"
#include "thread.h"
#include <sdbus-c++/sdbus-c++.h>
#include <fstream>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    void playpause();
    void stop();
    void seek();
    void save();
    void load();
    void checkChanged(int state);
    void updateTrackInfo();
    void sliderChanged();
    void setAlbumArtwork(QPixmap *);
    void bpmChanged(int bpm);

    ~MainWindow();
    TimeLine * timeline;


private:


    Ui::MainWindow *ui;
    WorkerThread *workerThread;
    QLabel *titleLabel, *artistLabel, *imageLabel, *remainingLabel, *pastLabel;
    QSlider *progressSlider;
    QCheckBox *checkbox;
    QSpinBox *bpm;


};
#endif // MAINWINDOW_H
