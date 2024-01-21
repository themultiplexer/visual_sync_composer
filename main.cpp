
#include "downloader.h"
#include "mainwindow.h"
#include "thread.h"

#include <QGuiApplication>
#include <QApplication>
#include <QtNetwork/QNetworkReply>
#include <QThread>
#include <QGraphicsScene>
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
#include <QThread>

using namespace std;

MainWindow *mainWindow;

void WorkerThread::run() {
    while (!isInterruptionRequested()) {
        msleep(1000);
        mainWindow->updateTrackInfo();

    }
}

void Downloader::downloadFinished(QNetworkReply *reply) {
    QPixmap pm;
    pm.loadFromData(reply->readAll());
    mainWindow->setAlbumArtwork(&pm);
    reply->deleteLater();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    mainWindow = new MainWindow();
    mainWindow->setWindowTitle("Hello World");
    mainWindow->resize(1920, 1080);

    // Show the main window
    mainWindow->show();
    return a.exec();
}
