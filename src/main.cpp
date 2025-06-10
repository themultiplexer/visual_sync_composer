#include <GL/glew.h>
#include "audiowindow.h"
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

std::array<uint8_t, 6> my_mac = {0xDC, 0x4E, 0xF4, 0x0A, 0x3F, 0x9F};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle("fusion");

    QPalette palette = QPalette();
    palette.setColor(QPalette::Window, QColor(53, 53, 53));
    palette.setColor(QPalette::WindowText, Qt::white);
    palette.setColor(QPalette::Base, QColor(25, 25, 25));
    palette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    palette.setColor(QPalette::ToolTipBase, Qt::black);
    palette.setColor(QPalette::ToolTipText, Qt::white);
    palette.setColor(QPalette::Text, Qt::white);
    palette.setColor(QPalette::Button, QColor(53, 53, 53));
    palette.setColor(QPalette::ButtonText, Qt::white);
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::Link, QColor(42, 130, 218));
    palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    palette.setColor(QPalette::HighlightedText, Qt::black);
    a.setPalette(palette);

    WifiEventProcessor *ep = new WifiEventProcessor(my_mac, "wlxdc4ef40a3f9f");

    AudioWindow *audioWindow = new AudioWindow(ep);
    audioWindow->setWindowTitle("Hello World");
    audioWindow->resize(1920, 1080);
    audioWindow->show();
    /*
    MainWindow *mainWindow = new MainWindow(ep);
    mainWindow->setWindowTitle("Hello World");
    mainWindow->resize(1920, 1080);
    mainWindow->show();
    */
    return a.exec();
}
