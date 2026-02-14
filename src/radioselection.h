#ifndef RADIOSELECTION_H
#define RADIOSELECTION_H

#include <QMainWindow>
#include <QWidget>
#include <QCloseEvent>
#include <QSplitter>
#include <QRadioButton>
#include <QMessageBox>
#include <QLabel>
#include <QSlider>
#include <QInputDialog>
#include <QLineEdit>
#include <QGroupBox>
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

class RadioSelection : public QGroupBox
{
    Q_OBJECT


public:
    RadioSelection(std::string title, std::vector<std::string> selection, std::function<void (int)> callback, int defaultValue, QWidget *parent);
};

#endif // RADIOSELECTION_H
