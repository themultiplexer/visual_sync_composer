#include "mainwindow.h"
#include "ProjectModel.h"
#include "gltext.h"
#include "wifieventprocessor.h"
#include <boost/circular_buffer.hpp>
#include <QRadioButton>
#include "netdevice.h"
#include "helper.h"


void WorkerThread::run() {
    while (!isInterruptionRequested()) {
        msleep(250);
        //mainWindow->updateTrackInfo();
    }
}

void Downloader::downloadFinished(QNetworkReply *reply) {
    QPixmap pm;
    pm.loadFromData(reply->readAll());
    //mainWindow->setAlbumArtwork(&pm);
    reply->deleteLater();
}

MainWindow::MainWindow(WifiEventProcessor *ep, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    GLText *test = new GLText(this);
    test->setMinimumHeight(100);
    this->ep->registerReceiver((WifiTextEventReceiver *)test);

    timeline = new TimeLine(ep);
    timeline->setMaximumHeight(200);
    timeline->addItem(1.0, 1.0, 0, "Fuck", QColor(255,0,0));

    // Create menu bar and actions
    QMenuBar *menuBar = new QMenuBar(this);
    QMenu *fileMenu = menuBar->addMenu("File");

    QAction *openAction = new QAction("Open", this);
    QObject::connect(openAction, &QAction::triggered, [&]() {
        std::cout << "Open action triggered";
    });
    fileMenu->addAction(openAction);

    QAction *exitAction = new QAction("Exit", this);
    QObject::connect(exitAction, &QAction::triggered, [&]() {
        qApp->quit();
    });
    fileMenu->addAction(exitAction);

    this->setMenuBar(menuBar);

    // Create central widget and set up layout
    QWidget* centralWidget = new QWidget(this);
    this->setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    imageLabel = new QLabel("Title", centralWidget);
    imageLabel->setMaximumHeight(50);
    imageLabel->setMaximumWidth(50);
    imageLabel->setScaledContents( true );
    titleLabel = new QLabel("Title", centralWidget);
    titleLabel->setFont(QFont("Arial", 20, QFont::Bold));
    artistLabel = new QLabel("Artist", centralWidget);

    QPushButton *playPauseButton = new QPushButton("Play/Pause", centralWidget);
    QPushButton *stopButton = new QPushButton("Stop", centralWidget);
    QPushButton *seekButton = new QPushButton("Seek", centralWidget);

    QHBoxLayout* mediaButtonsLayout = new QHBoxLayout(centralWidget);
    mediaButtonsLayout->addWidget(stopButton);
    mediaButtonsLayout->addWidget(playPauseButton);
    mediaButtonsLayout->addWidget(seekButton);

    QPushButton *saveButton = new QPushButton("Save", centralWidget);
    QPushButton *loadButton = new QPushButton("Load", centralWidget);
    QPushButton *loadLyricsButton = new QPushButton("Load Lyrics", centralWidget);
    QHBoxLayout* buttonLayout = new QHBoxLayout(centralWidget);
    buttonLayout->addWidget(loadButton);
    buttonLayout->addWidget(loadLyricsButton);
    buttonLayout->addWidget(saveButton);

    QHBoxLayout* sliderLayout = new QHBoxLayout(centralWidget);
    progressSlider = new QSlider(Qt::Horizontal, centralWidget);
    progressSlider->setRange(0, 100);
    connect(progressSlider, &QSlider::sliderReleased, this, &MainWindow::sliderChanged);
    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    pastLabel = new QLabel("00:00", centralWidget);
    pastLabel->setFont(font);
    remainingLabel = new QLabel("00:00", centralWidget);
    remainingLabel->setFont(font);
    sliderLayout->addWidget(pastLabel);
    sliderLayout->addWidget(progressSlider);
    sliderLayout->addWidget(remainingLabel);

    QHBoxLayout* toolLayout = new QHBoxLayout(centralWidget);
    checkbox = new QCheckBox(centralWidget);
    checkbox->setText("Follow Time");
    connect(checkbox, &QCheckBox::stateChanged, this, &MainWindow::checkChanged);
    toolLayout->addWidget(checkbox);
    bpm = new QSpinBox();
    bpm->setMinimum(1);
    bpm->setMaximumWidth(100);
    bpm->setMaximum(500);
    toolLayout->addStretch(1);
    toolLayout->addWidget(new QLabel("BPM"));
    toolLayout->addWidget(bpm);
    QSpinBox * snapInterval = new QSpinBox();
    snapInterval->setMinimum(1);
    snapInterval->setValue(timeline->getGridSnapInterval());
    snapInterval->setMaximum(4);
    snapInterval->setMaximumWidth(100);
    toolLayout->addWidget(new QLabel("Snap"));
    toolLayout->addWidget(snapInterval);

    connect(bpm, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::spinboxChanged);
    connect(snapInterval, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::spinboxChanged);

    // Create a layout
    QWidget *gridWidget = new QWidget;
    QGridLayout *gridLayout = new QGridLayout;

    // Loop to create buttons and add them to the layout
    for (int row = 0; row < 10; ++row) {
        for (int col = 0; col < 10; ++col) {
            QPushButton *button = new QPushButton(QString("Button %1").arg(row * 10 + col + 1));
            gridLayout->addWidget(button, row, col);
        }
    }
    gridWidget->setLayout(gridLayout);

    // Add widgets to the layout
    mainLayout->addWidget(test);
    mainLayout->addWidget(imageLabel);
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(artistLabel);
    mainLayout->addLayout(mediaButtonsLayout);
    mainLayout->addLayout(sliderLayout);
    mainLayout->addLayout(toolLayout);
    mainLayout->addWidget(timeline->view);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(gridWidget);

    timeline->update();


    // Set up connections for buttons to slots
    connect(playPauseButton, &QPushButton::clicked, this, &MainWindow::playpause);
    connect(stopButton, &QPushButton::clicked, this, &MainWindow::stop);
    connect(seekButton, &QPushButton::clicked, this, &MainWindow::seek);

    connect(saveButton, &QPushButton::clicked, this, &MainWindow::save);
    connect(loadButton, &QPushButton::clicked, this, &MainWindow::load);
    connect(loadLyricsButton, &QPushButton::clicked, this, &MainWindow::loadLyrics);

    bpm->setValue(120);

    workerThread = new WorkerThread();
    connect(workerThread, &QThread::started, workerThread, &WorkerThread::onThreadStarted);
    workerThread->start();
}

std::string currentUrl = "";

void MainWindow::updateTrackInfo() {
    const char* destinationName = "org.mpris.MediaPlayer2.spotify";
    const char* objectPath = "/org/mpris/MediaPlayer2";
    auto c = sdbus::createSessionBusConnection();
    auto concatenatorProxy = sdbus::createProxy(*c, destinationName, objectPath);
    std::map<std::string, sdbus::Variant> metadata = concatenatorProxy->getProperty("Metadata").onInterface("org.mpris.MediaPlayer2.Player");

    auto test = metadata["mpris:artUrl"].get<std::string>();

    if (test != currentUrl) {
        emit workerThread->startDownload(QString(test.c_str()));
        currentUrl = test;
        std::cout << "Updating Cover: " << test << std::endl;
    }

    uint64_t track_length = metadata["mpris:length"].get<uint64_t>();
    progressSlider->setMaximum(static_cast<double>(track_length) / 1e6);
    timeline->setTrackTime(static_cast<double>(track_length) / 1e6);
    remainingLabel->setText(QString(std::to_string(static_cast<double>(track_length) / 1e6).c_str()));

    if (metadata.count("xesam:artist")) {
        // Check if the "xesam:artist" value is a vector of strings
        if (metadata["xesam:artist"].containsValueOfType<std::vector<std::string>>()) {
            // Check if the vector is not empty
            auto artistVector = metadata["xesam:artist"].get<std::vector<std::string>>();
            if (!artistVector.empty()) {
                std::string artists;
                for (std::string artist : artistVector) {
                    artists += artist;
                }
                artistLabel->setText(QString(artists.c_str()));
            } else {
                std::cerr << "Error: Artist vector is empty." << std::endl;
            }
        } else {
            std::cerr << "Error: 'xesam:artist' value is not a vector of strings." << std::endl;
        }
    } else {
        std::cerr << "Error: 'xesam:artist' key not found in metadata." << std::endl;
    }

    if (metadata.count("xesam:title")) {
        // Check if the "xesam:title" value is a vector of strings
        if (metadata["xesam:title"].containsValueOfType<std::string>()) {
            auto title = metadata["xesam:title"].get<std::string>();
            titleLabel->setText(QString(title.c_str()));
        } else {
            std::cerr << "Error: 'xesam:title' value is not a string." << std::endl;
        }
    } else {
        std::cerr << "Error: 'xesam:title' key not found in metadata." << std::endl;
    }

    std::string playbackStatus = concatenatorProxy->getProperty("PlaybackStatus").onInterface("org.mpris.MediaPlayer2.Player").get<std::string>();

    if(playbackStatus == "Playing") {
        timeline->setPaused(false);
    } else if(playbackStatus == "Paused") {
        timeline->setPaused(true);
    }


    int64_t position = concatenatorProxy->getProperty("Position").onInterface("org.mpris.MediaPlayer2.Player").get<int64_t>();
    if (!progressSlider->isSliderDown()) {
        progressSlider->setSliderPosition(static_cast<double>(position) / 1e6);
    }

    pastLabel->setText(QString(std::to_string(static_cast<double>(position) / 1e6).c_str()));
    timeline->setTime(static_cast<double>(position) / 1e6);
}

void MainWindow::playpause() {
    std::cout << "Clicked " << std::endl;
    const char* destinationName = "org.mpris.MediaPlayer2.spotify";
    const char* objectPath = "/org/mpris/MediaPlayer2";
    auto c = sdbus::createSessionBusConnection();
    auto concatenatorProxy = sdbus::createProxy(*c, destinationName, objectPath);
    concatenatorProxy->callMethod("PlayPause").onInterface("org.mpris.MediaPlayer2.Player");
}

void MainWindow::stop() {
    std::cout << "Clicked " << std::endl;
    const char* destinationName = "org.mpris.MediaPlayer2.spotify";
    const char* objectPath = "/org/mpris/MediaPlayer2";
    auto c = sdbus::createSessionBusConnection();
    auto concatenatorProxy = sdbus::createProxy(*c, destinationName, objectPath);
    concatenatorProxy->callMethod("Stop").onInterface("org.mpris.MediaPlayer2.Player");
}

void MainWindow::seek() {
    std::cout << "Clicked " << std::endl;
    const char* destinationName = "org.mpris.MediaPlayer2.spotify";
    const char* objectPath = "/org/mpris/MediaPlayer2";
    auto c = sdbus::createSessionBusConnection();
    auto concatenatorProxy = sdbus::createProxy(*c, destinationName, objectPath);
    concatenatorProxy->callMethod("Seek").onInterface("org.mpris.MediaPlayer2.Player").withArguments((int64_t)-1000000);
}
void MainWindow::sliderChanged() {
    if (sender() == progressSlider) {
        std::cout << "Slider Changed " << progressSlider->value() << std::endl;
        const char* destinationName = "org.mpris.MediaPlayer2.spotify";
        const char* objectPath = "/org/mpris/MediaPlayer2";
        auto c = sdbus::createSessionBusConnection();
        auto concatenatorProxy = sdbus::createProxy(*c, destinationName, objectPath);
        std::map<std::string, sdbus::Variant> metadata = concatenatorProxy->getProperty("Metadata").onInterface("org.mpris.MediaPlayer2.Player");
        auto trackid = metadata["mpris:trackid"].get<std::string>();
        auto trackObject = sdbus::ObjectPath(trackid);
        concatenatorProxy->callMethod("SetPosition").onInterface("org.mpris.MediaPlayer2.Player").withArguments(trackObject, (int64_t)(progressSlider->value() * 1e6));
        timeline->setTimeImpl(progressSlider->value());
        timeline->buildEventList();
    }
}

void MainWindow::save() {
    QString filePath = QFileDialog::getSaveFileName(this, tr("Load Composition"), QDir::currentPath(), tr("Visual Sync Composition (*.vsc)"));

    if (!filePath.endsWith(".vsc")) {
        filePath.append(".vsc");
    }

    ProjectModel project;
    project.bpm = bpm->value();
    project.name = "LOL WTF";

    auto tracks = timeline->getTracks();

    for (auto track : tracks) {
        project.events.push_back(EventModel(track));
    }

    std::ofstream ofs(filePath.toStdString(), std::ios::binary);
    boost::archive::binary_oarchive oa(ofs);
    oa & project;

    std::cout << project;
}

void MainWindow::load() {
    QString filePath = QFileDialog::getOpenFileName(this, tr("Load Composition"), QDir::currentPath(), tr("Visual Sync Composition (*.vsc)"));

    if (!filePath.isEmpty()) {
        qDebug() << "gotFile";

        timeline->clear();

        ProjectModel project;
        std::ifstream ifs(filePath.toStdString(), std::ios::binary);
        boost::archive::binary_iarchive ia(ifs);
        ia >> project;

        bpm->setValue(project.bpm);

        for (EventModel e : project.events) {
            timeline->addItem(e.start, e.duration, e.lane, e.text, QColor(255,0,0));
        }

        timeline->buildEventList();

        std::cout << project;
    }
}

void MainWindow::loadLyrics() {
    QString filePath = QFileDialog::getOpenFileName(this, tr("Load Composition"), QDir::currentPath(), tr("Text (*.txt)"));

    if (!filePath.isEmpty()) {
        std::ifstream file;
        file.open(filePath.toStdString());
        if (!file.is_open()) return;
        std::string aword;
        std::vector<std::string> words;
        while (file >> aword)
        {
            words.push_back(aword);
        }

        for (int i = 0; i < words.size(); ++i) {
            timeline->addItem(((float)i/words.size()) * 150.0, (float)words[i].size() / 10.0, i % 4, words[i], QColor(255,0,0));
        }
    }
}

void MainWindow::checkChanged(int state)
{
    timeline->setFollowTime(checkbox->isChecked());
}


void MainWindow::closeEvent (QCloseEvent *event)
{
    QMessageBox::StandardButton resBtn = QMessageBox::question( this, "APP_NAME",
                                                               tr("Are you sure?\n"),
                                                               QMessageBox::No | QMessageBox::Yes,
                                                               QMessageBox::Yes);
    if (resBtn != QMessageBox::Yes) {
        event->ignore();
    } else {
        event->accept();
    }
}


void MainWindow::setAlbumArtwork(QPixmap *artwork) {
    imageLabel->setPixmap(*artwork);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::spinboxChanged(int value)
{
    if (sender() == bpm) {
        timeline->setBpm(value);
    } else {
        timeline->setGridSnapInterval(value);
    }

}
