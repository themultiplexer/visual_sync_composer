#include "audiowindow.h"
#include "fullscreenwindow.h"
#include "horizontal_line.h"
#include "mdnsflasher.h"
#include "tubewidget.h"
#include "devicereqistry.h"
#include <QDockWidget>

#define USE_DOCK 0

AudioWindow::AudioWindow(WifiEventProcessor *ep, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), popoutGlv(nullptr), fullScreenWindow(new FullscreenWindow())
{
    this->ep = ep;
    ui->setupUi(this);
    //NetDevice h = NetDevice("wlxdc4ef40a3f9f");
    //h.setInterface(false);
    //h.enableMonitorMode();
    //h.setInterface(true);

    ep->initHandlers();
    //showFullScreen();

    rng = new std::mt19937(dev());
    hueRandom = new std::uniform_int_distribution<std::mt19937::result_type>(0, 255);

    for (auto color : colors) {
        color = (*hueRandom)(*rng);
    }

    setWindowFlags(Qt::Window | Qt::WindowFullscreenButtonHint);
    //setWindowState(Qt::WindowMaximized);

    // Create menu bar and actions
    QMenuBar *menuBar = new QMenuBar(this);
    QMenu *fileMenu = menuBar->addMenu("File");
    QMenu *viewMenu = menuBar->addMenu("View");

    QAction *openAction = new QAction("Open", this);
    QObject::connect(openAction, &QAction::triggered, [&]() {
        std::cout << "Open action triggered";
    });
    fileMenu->addAction(openAction);

    QAction *fullscreenAction = new QAction("Fullscreen", this);
    fullscreenAction->setCheckable(true);
    QObject::connect(fullscreenAction, &QAction::triggered, [&](bool checked) {
        if(checked) {
            qDebug() << "Fullscreen";

            if (USE_DOCK) {
                dock->setFloating(true);
                QWidget* floatingWindow = dock->window();
                floatingWindow->raise();            // Brings the window to the front
                floatingWindow->activateWindow();   // Ensures the window is focused

                QTimer::singleShot(16, dock, [=]() {
                    dock->update();  // Request an update every 16ms (~60 FPS)
                });
            } else {
                this->hide();
                popoutGlv = new OGLWidget(1024, fullScreenWindow);
                popoutGlv->setRegions(glv->getRegions());
                fullScreenWindow->setCentralWidget(popoutGlv);
                connect(popoutGlv, &OGLWidget::valueChanged, this, &AudioWindow::sliderChanged);
                fullScreenWindow->showFullScreen();

                QObject::connect(fullScreenWindow, &FullscreenWindow::escapePressed, [&]() {
                    this->show();
                    fullScreenWindow->close();
                    glv->setRegions(popoutGlv->getRegions());
                });
            }
        } else {
            if (USE_DOCK) {
                qDebug() << "Exit Fullscreen";
                dock->window()->showNormal();
                dock->setFloating(false);
                addDockWidget(Qt::TopDockWidgetArea, dock);
            }
        }
    });
    viewMenu->addAction(fullscreenAction);

    QSurfaceFormat format;
    format.setOption(QSurfaceFormat::DebugContext);
    QSurfaceFormat::setDefaultFormat(format);

    QAction *exitAction = new QAction("Exit", this);
    QObject::connect(exitAction, &QAction::triggered, [&]() {
        qApp->quit();
    });
    fileMenu->addAction(exitAction);

    this->setMenuBar(menuBar);

    QSplitter* mainLayout = new QSplitter(Qt::Vertical, this);
    this->setCentralWidget(mainLayout);
    mainLayout->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);
    //splitter->setStretchFactor(0, 0);

    QWidget *modesWidget = new QWidget;
    modesWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    QHBoxLayout* modesLayout = new QHBoxLayout(modesWidget);
    std::vector<QRadioButton*> ledModeRadioButtons;
    for (std::string effect : values) {
        QRadioButton *radio = new QRadioButton();
        radio->setText(effect.c_str());
        connect(radio, &QRadioButton::toggled, this, &AudioWindow::effectChanged);
        ledModeRadioButtons.push_back(radio);
        modesLayout->addWidget(radio);
    }

    QWidget *modifiersWidget = new QWidget;
    modifiersWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    QHBoxLayout* modifiersLayout = new QHBoxLayout(modifiersWidget);
    modifiersLayout->addWidget(new QLabel("Modifiers:"));
    for (std::string effect : {"Fadeout After Peak","2","3","4","5","6","7","8"}) {
        QCheckBox *check = new QCheckBox();
        check->setText(effect.c_str());
        connect(check, &QCheckBox::toggled, this, &AudioWindow::modifierChanged);
        ledModifierCheckboxes.push_back(check);
        modifiersLayout->addWidget(check);
    }
    modifiersLayout->addStretch();

    QWidget *tubesWidget = new QWidget;
    tubesWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    QHBoxLayout* tubesLayout = new QHBoxLayout(tubesWidget);
    auto macs = devicereqistry::macs();
    for (auto mac : macs) {
        VSCTube *tube = new VSCTube(QString(arrayToHexString(mac).c_str()), this);
        tube->setMaximumWidth(250);
        connect(tube, &VSCTube::valueChanged, this, [=, this](){
            std::vector<int> offsets;
            for (auto t : tubes) {
                offsets.push_back(t->value());
            }
            ep->setTubeOffsets(offsets);
            ep->sendConfig();
        });

        connect(tube, &VSCTube::flashClicked, this, [=](){
            auto it = std::find(tubes.begin(), tubes.end(), tube);
            if (it != tubes.end()) {
                int index = std::distance(tubes.begin(), it);
                ep->sendUpdateMessageTo(macs[index]);
            }
        });

        connect(tube, &VSCTube::buttonPressed, this, [=](bool right){
            auto it = std::find(tubes.begin(), tubes.end(), tube);
            if (it != tubes.end()) {
                int index = std::distance(tubes.begin(), it);
                if (right) {
                    if (index < tubes.size() - 1) {
                        std::swap(tubes[index], tubes[index + 1]);
                    }
                } else {
                    if (index > 0) {
                        std::swap(tubes[index], tubes[index - 1]);
                    }
                }
            }

            while (tubesLayout->count() > 0) {
                QWidget *widget = tubesLayout->itemAt(0)->widget();
                tubesLayout->removeWidget(widget);
            }

            for (auto t : tubes) {
                tubesLayout->addWidget(t);
            }

        });
        tubes.push_back(tube);
        tubesLayout->addWidget(tube, 1);
    }

    sensitivitySlider = new VSCSlider("Sensitivity", Qt::Horizontal, tubesWidget);
    sensitivitySlider->setMinimum(0);
    sensitivitySlider->setValue(80);
    sensitivitySlider->setMaximum(100);
    connect(sensitivitySlider, &VSCSlider::valueChanged, this, &AudioWindow::sliderChanged);

    saturationSlider = new VSCSlider("Saturation", Qt::Horizontal, tubesWidget);
    saturationSlider->setMinimum(0);
    saturationSlider->setValue(50);
    saturationSlider->setMaximum(255);
    connect(saturationSlider, &VSCSlider::valueChanged, this, &AudioWindow::sliderChanged);

    brightnessSlider = new VSCSlider("Brightness", Qt::Horizontal, tubesWidget);
    brightnessSlider->setMinimum(0);
    brightnessSlider->setValue(0);
    brightnessSlider->setMaximum(255);
    connect(brightnessSlider, &VSCSlider::sliderReleased, this, &AudioWindow::sliderChanged);

    speedSlider = new VSCSlider("Speed", Qt::Horizontal, tubesWidget);
    speedSlider->setMinimum(1);
    speedSlider->setValue(5);
    speedSlider->setMaximum(255);
    connect(speedSlider, &VSCSlider::sliderReleased, this, &AudioWindow::sliderChanged);

    effect1Slider = new VSCSlider("Param 1", Qt::Horizontal, tubesWidget);
    effect1Slider->setMinimum(1);
    effect1Slider->setValue(5);
    effect1Slider->setMaximum(255);
    connect(effect1Slider, &VSCSlider::sliderReleased, this, &AudioWindow::sliderChanged);

    effect2Slider = new VSCSlider("Param 2", Qt::Horizontal, tubesWidget);
    effect2Slider->setMinimum(1);
    effect2Slider->setValue(5);
    effect2Slider->setMaximum(255);
    connect(effect2Slider, &VSCSlider::sliderReleased, this, &AudioWindow::sliderChanged);

    effect3Slider = new VSCSlider("Param 3", Qt::Horizontal, tubesWidget);
    effect3Slider->setMinimum(1);
    effect3Slider->setValue(128);
    effect3Slider->setMaximum(255);
    connect(effect3Slider, &VSCSlider::sliderReleased, this, &AudioWindow::sliderChanged);

    effect4Slider = new VSCSlider("Param 4", Qt::Horizontal, tubesWidget);
    effect4Slider->setMinimum(1);
    effect4Slider->setValue(128);
    effect4Slider->setMaximum(255);
    connect(effect4Slider, &VSCSlider::sliderReleased, this, &AudioWindow::sliderChanged);

    QWidget *slidersWidget = new QWidget;
    QVBoxLayout* slidersLayout = new QVBoxLayout(slidersWidget);
    slidersLayout->addStretch();
    slidersLayout->addWidget(sensitivitySlider);
    slidersLayout->addWidget(brightnessSlider);
    slidersLayout->addWidget(saturationSlider);
    slidersLayout->addWidget(speedSlider);
    slidersLayout->addWidget(effect1Slider);
    slidersLayout->addWidget(effect2Slider);
    slidersLayout->addWidget(effect3Slider);
    slidersLayout->addWidget(effect4Slider);
    slidersLayout->addStretch();
    slidersLayout->setSpacing(0);

    QWidget *bottomWidget = new QWidget;
    bottomWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);
    QHBoxLayout *bottomLayout = new QHBoxLayout(bottomWidget);
    QGridLayout *gridLayout = new QGridLayout;
    std::vector<EffectPresetModel *> presets = EffectPresetModel::readJson("effects.json");
    std::vector<EffectPresetButton*> buttons{};
    // Loop to create buttons and add them to the layout
    for (int row = 0; row < 10; ++row) {
        for (int col = 0; col < 10; ++col) {
            EffectPresetButton *button = new EffectPresetButton(presets[row * 10 + col], this);
            gridLayout->addWidget(button, row, col);
            connect(button, &EffectPresetButton::releasedInstantly, [=](){
                EffectPresetModel *model = button->getModel();
                this->brightnessSlider->setValue(model->config.brightness);
                this->speedSlider->setValue(model->config.speed_factor);
                this->effect1Slider->setValue(model->config.parameter1);
                this->effect2Slider->setValue(model->config.parameter2);
                this->effect3Slider->setValue(model->config.parameter3);
                this->saturationSlider->setValue(model->config.sat);
                for (int i = 0; i < 8; ++i) {
                    this->ledModifierCheckboxes[i]->blockSignals(true);
                    this->ledModifierCheckboxes[i]->setChecked(((model->config.modifiers >> (7 - i)) & 0x01));
                    this->ledModifierCheckboxes[i]->blockSignals(false);
                }
                ledModeRadioButtons[model->config.led_mode]->blockSignals(true);
                ledModeRadioButtons[model->config.led_mode]->setChecked(true);
                ledModeRadioButtons[model->config.led_mode]->blockSignals(false);

                ep->setMasterconfig(model->config);
                ep->sendConfig();
            });
            connect(button, &EffectPresetButton::longPressed, [=](){
                bool ok;
                QString text = QInputDialog::getText(this, tr("QInputDialog::getText()"),
                                                     tr("User name:"), QLineEdit::Normal,
                                                     "", &ok);
                if (ok && !text.isEmpty()) {
                    EffectPresetModel *model = button->getModel();
                    model->config = ep->getMasterconfig();
                    model->setName(text);
                    button->setModel(model);
                    EffectPresetModel::saveToJsonFile(presets, "effects.json");
                }
            });
            buttons.push_back(button);
        }
    }
    bottomLayout->addLayout(slidersLayout);
    bottomLayout->addLayout(gridLayout);


    QHBoxLayout *header = new QHBoxLayout();
    bpmLabel = new QLabel("bpm");
    tmpLabel = new QLabel("name");
    header->addWidget(bpmLabel);
    header->addWidget(tmpLabel);


    glv = new OGLWidget(1024);
    glv->setMinimumHeight(100);
    connect(glv, &OGLWidget::valueChanged, this, &AudioWindow::sliderChanged);

    // Create a dock widget to hold it
    if (USE_DOCK) {
        dock = new QDockWidget("OpenGL View", this);
        dock->setWidget(glv);
        dock->setAllowedAreas(Qt::TopDockWidgetArea);
    }

    QWidget *firmwareWidget = new QWidget;
    QHBoxLayout *fwButtons = new QHBoxLayout(firmwareWidget);

    QPushButton *fwbutton = new QPushButton("Put All in Firmwareupdate Mode");
    connect(fwbutton, &QPushButton::pressed, [=](){
        ep->sendUpdateMessage();
    });

    QPushButton *flashbutton = new QPushButton("FW Update");
    connect(flashbutton, &QPushButton::pressed, [=](){
        mdnsflasher::flash("../LEDTube.ino.bin");
    });

    fwButtons->addWidget(fwbutton);
    fwButtons->addWidget(flashbutton);

    QPushButton *syncbutton = new QPushButton("Sync");
    connect(syncbutton, &QPushButton::pressed, [=](){
        ep->sendSync();
    });

    QWidget *glvWidget = new QWidget;
    QHBoxLayout *frequencyLayout = new QHBoxLayout(glvWidget);
    timeSlider = new VSCSlider("Dcy", Qt::Vertical, glvWidget);
    timeSlider->setMinimum(1);
    timeSlider->setValue(50);
    timeSlider->setMaximum(100);
    connect(timeSlider, &VSCSlider::sliderReleased, this, &AudioWindow::sliderChanged);

    otherSlider = new VSCSlider("Oth", Qt::Vertical, glvWidget);
    otherSlider->setMinimum(1);
    otherSlider->setValue(50);
    otherSlider->setMaximum(100);
    connect(otherSlider, &VSCSlider::sliderReleased, this, &AudioWindow::sliderChanged);

    frequencyLayout->addWidget(timeSlider);
    if (USE_DOCK) {
        frequencyLayout->addWidget(dock);
    } else {
        frequencyLayout->addWidget(glv);
    }
    frequencyLayout->addWidget(otherSlider);

    // Add widgets to the layout
    mainLayout->addWidget(tubesWidget);
    mainLayout->addWidget(glvWidget);
    mainLayout->addWidget(modesWidget);
    mainLayout->addWidget(modifiersWidget);
    mainLayout->addWidget(bottomWidget);
    mainLayout->addWidget(firmwareWidget);
    mainLayout->addWidget(syncbutton);

    if (getuid() == 0) {
        printf("Dropping privs\n");
        /* process is running as root, drop privileges */
        if (setgid(1000) != 0)
            printf("setgid: Unable to drop group privileges: %s\n", strerror(errno));
        if (setuid(1000) != 0)
            printf("setuid: Unable to drop user privileges: %s\n", strerror(errno));
    }
    setenv("HOME", "/home/josh/", 1);
    setenv("USER", "josh", 1);
    setenv("XDG_RUNTIME_DIR", "/run/user/1000", 1);
    printf("%d\n", getuid());
    printf("%s\n", getenv("USER"));

    a = new AudioAnalyzer();
    a->getdevices();
    a->startRecording();

    QTimer *qtimer = new QTimer(this);
    connect(qtimer, &QTimer::timeout, this, &AudioWindow::checkTime);
    qtimer->start(1);
}

void AudioWindow::checkTime(){
    auto fl = a->getLeftFrequencies();
    auto fr = a->getRightFrequencies();

    for (int i = 0; i < FRAMES/2; i++) {
        fl[i] *= log10(((float)i/(FRAMES/2)) * 5 + 1.01);
        fl[i] = log10(fl[i] * 2.0 + 1.01);

        fr[i] *= log10(((float)i/(FRAMES/2)) * 5 + 1.01);
        fr[i] = log10(fr[i] * 2.0 + 1.01);
    }

    OGLWidget *w = glv;
    if (fullScreenWindow->isActiveWindow()) {
        w = popoutGlv;
    }

    w->processData(fl, [this](FrequencyRegion &region){
        if (region.getScaledMax() < 1000) {
            uint64_t selectedHue = colors.front();
            ep->peakEvent(selectedHue);
            std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
            beats.push(region.getBeatMillis());

            for (auto t : tubes) {
                t->setPeaked({(float) selectedHue / (float) 255, saturationSlider->pct(), 1.0});
            }

            colors.push((*hueRandom)(*rng));
        }

        float sum = std::accumulate(beats.begin(), beats.end(), 0.0);
        float mean = sum / beats.size();
        bpmLabel->setText(std::to_string((int)std::round(60.0 / (mean / 1000.0))).c_str());
        tmpLabel->setText(std::to_string(mean).c_str());
    });

    w->setFrequencies(fl, fr);

    for (auto t : tubes) {
        t->updateGL();
    }
}

void AudioWindow::sliderChanged()
{
    if (sender() == glv) {
        sensitivitySlider->setValue(glv->getThresh() * 100);
    }
    if (sender() == timeSlider) {
        glv->setDecay(timeSlider->pct() * 0.1);
    }
    if (sender() == sensitivitySlider) {
        glv->setThresh(sensitivitySlider->pct());
    }

    CONFIG_DATA d = ep->getMasterconfig();
    if (sender() == brightnessSlider) {
        d.brightness = brightnessSlider->value();
    } else if (sender() == saturationSlider) {
        d.sat = saturationSlider->value();
    } else if (sender() == speedSlider) {
        d.speed_factor = speedSlider->value();
    } else if (sender() == effect1Slider) {
        d.parameter1 = effect1Slider->value();
    } else if (sender() == effect2Slider) {
        d.parameter2 = effect2Slider->value();
    } else if (sender() == effect3Slider) {
        d.parameter3 = effect3Slider->value();
    } else if (sender() == effect4Slider) {
        d.offset = effect4Slider->value();
    } else if (sender() == otherSlider) {
    }
    ep->setMasterconfig(d);
    ep->sendConfig();
}

void AudioWindow::effectChanged(bool state)
{
    if(state) {
        QRadioButton* button = qobject_cast<QRadioButton*>(sender());
        auto it = std::find(std::begin(values), std::end(values), button->text().toStdString());
        int index = it - std::begin(values);
        CONFIG_DATA d = ep->getMasterconfig();
        d.led_mode = index;
        ep->setMasterconfig(d);
        ep->sendConfig();
    }
}

void AudioWindow::modifierChanged(bool state)
{
    std::vector<uint8_t> bits;
    for (auto ch : ledModifierCheckboxes) {
        bits.push_back(ch->isChecked() ? 1 : 0);
    }
    qDebug() << bits << " " << bitsToBytes(bits.data());
    CONFIG_DATA d = ep->getMasterconfig();
    d.modifiers = bitsToBytes(bits.data());
    ep->setMasterconfig(d);
    ep->sendConfig();
}

void AudioWindow::closeEvent(QCloseEvent *event)
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

AudioWindow::~AudioWindow()
{
    delete ui;
}
