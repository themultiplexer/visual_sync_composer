#include "audiowindow.h"
#include "audiofilter.h"
#include "fullscreenwindow.h"
#include "mdnsflasher.h"
#include "devicereqistry.h"
#include <QDockWidget>

#define USE_DOCK 0

AudioWindow::AudioWindow(WifiEventProcessor *ep, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), popoutGlv(nullptr), activeEffectPresetButton(nullptr), activeTubePresetButton(nullptr), fullScreenWindow(new FullscreenWindow()), wifiLabel(nullptr), currentEffect(-1), currentPreset(-1), timer(nullptr), tubeFrames(0)
{
    numBeats = 0;
    numGroups = 1;

    this->ep = ep;
    ui->setupUi(this);
    NetDevice h = NetDevice("wlxdc4ef40a3f9f");
    h.setInterface(false);
    h.enableMonitorMode();
    h.setInterface(true);
    lastColorRed = false;

    effectPresets = PresetModel::readJson<EffectPresetModel>("effects.json");
    tubePresets = PresetModel::readJson<TubePresetModel>("tubes.json");

    ep->initHandlers();
    //showFullScreen();

    rng = new std::mt19937(dev());
    hueRandom = new std::uniform_int_distribution<std::mt19937::result_type>(0, 360);
    effectRandom = new std::uniform_int_distribution<std::mt19937::result_type>(0, 100);

    for (auto color : colors) {
        hsv col = {0.0, 1.0, 0.0};
        colors.push({0.0, 1.0});
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

                QTimer::singleShot(16, dock, [this]() {
                    dock->update();  // Request an update every 16ms (~60 FPS)
                });
            } else {
                this->hide();
                popoutGlv = new OGLWidget(1024, fullScreenWindow);
                popoutGlv->setRegions(glv->getRegions());
                fullScreenWindow->setCentralWidget(popoutGlv);
                connect(popoutGlv, &OGLWidget::threshChanged, this, &AudioWindow::sliderChanged);
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

    QWidget *superWidget = new QWidget(this);
    QVBoxLayout *superLayout = new QVBoxLayout();
    this->setCentralWidget(superWidget);

    QWidget *statusWidget = new QWidget(superWidget);
    statusWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    QHBoxLayout* statusLayout = new QHBoxLayout(statusWidget);
    statusLayout->addWidget(new QLabel("Status:"));
    wifiLabel = new QLabel("Offline");
    statusLayout->addWidget(wifiLabel);

    QSplitter* mainLayout = new QSplitter(Qt::Vertical, superWidget);

    superLayout->addWidget(statusWidget);
    superLayout->addWidget(mainLayout);
    mainLayout->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);

    if (false) {
        VSCTube *demotube = new VSCTube("asdsadasdsadsad", mainLayout);
        mainLayout->addWidget(demotube);
        timer = new QTimer(this);
        QObject::connect(timer, &QTimer::timeout, this, [demotube, this](){
            if (tubeFrames % 20 == 0) {
                hsv color = {(float)(*hueRandom)(*rng), 1.0, 1.0};
                demotube->setPeaked(hsv2rgb(color), 0);
            }
            tubeFrames++;
            demotube->updateGL();
        });
        timer->start(16);
    }




    QWidget *modesWidget = new QWidget;
    modesWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    QHBoxLayout* modesLayout = new QHBoxLayout(modesWidget);
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
    for (std::string effect : {"Fadeout After Peak","No Color Delay","Reversed","4","5","6","7","8"}) {
        QCheckBox *check = new QCheckBox();
        check->setText(effect.c_str());
        connect(check, &QCheckBox::toggled, this, &AudioWindow::modifierChanged);
        ledModifierCheckboxes.push_back(check);
        modifiersLayout->addWidget(check);
    }
    modifiersLayout->addStretch();

    QWidget *autoSelectorWidget = new QWidget;
    QHBoxLayout *autoSelectorLayout = new QHBoxLayout(autoSelectorWidget);
    autoSelectorLayout->addWidget(new QLabel("Auto Mode:"));

    for (std::string effect : {"Color", "Effect", "Composition"}) {
        QCheckBox *checkbox = new QCheckBox();
        checkbox->setText(effect.c_str());
        connect(checkbox, &QCheckBox::toggled, this, [=](){

        });
        autoSelectorLayout->addWidget(checkbox);
        autoCheckboxes.push_back(checkbox);
    }
    modifiersLayout->addWidget(autoSelectorWidget);



    QWidget *tubesWidget = new QWidget;
    //tubesWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    QHBoxLayout* tubesLayout = new QHBoxLayout(tubesWidget);
    auto macs = devicereqistry::macs();
    for (auto mac : macs) {
        VSCTube *tube = new VSCTube(arrayToHexString(mac), this);
        //tube->setMinimumHeight(250);
        tube->setMaximumWidth(250);
        connect(tube, &VSCTube::valueChanged, this, [=, this](){
            sendTubeSyncData();
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

    QWidget *presetsWidget = new QWidget;
    //tubesWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    QGridLayout* presetsLayout = new QGridLayout(presetsWidget);
    for (int row = 0; row < 5; ++row) {
        for (int col = 0; col < 5; ++col) {
            TubePresetModel *model = tubePresets[row * 10 + col];
            PresetButton *button = new PresetButton(model, this);
            presetsLayout->addWidget(button, row, col);
            connect(button, &PresetButton::releasedInstantly, [=](){
                ptrdiff_t index = std::distance(tubeButtons.begin(), std::find(tubeButtons.begin(), tubeButtons.end(), button));
                TubePresetModel *model = static_cast<TubePresetModel *>(button->getModel());
                if (currentPreset != -1) {
                    tubeButtons[currentPreset]->setStyleSheet("");
                }
                button->setStyleSheet("background-color: green");
                std::cout << "WTF" << std::endl;
                applyTubePreset(model);
                currentPreset = (int)index;
                activeTubePresetButton = button;
                sendTubeSyncData();

                for (auto t : tubes) {
                    t->sync();
                }
            });
            connect(button, &PresetButton::longPressed, [=](){
                bool ok;
                QString text = QInputDialog::getText(this, tr("Tube Config"), tr("Enter preset name:"), QLineEdit::Normal, "", &ok);
                if (ok && !text.isEmpty()) {

                    std::map<std::string, TubePreset> preset;
                    for (auto t : tubes) {
                        TubePreset p = TubePreset();
                        p.delay = t->getDelay();
                        p.group = t->getGroup();
                        preset[t->getMac()] = p;
                    }

                    TubePresetModel *model = static_cast<TubePresetModel *>(button->getModel());
                    model->setTubePresets(preset);
                    model->setName(text.toStdString());
                    button->setModel(model);
                    PresetModel::saveToJsonFile(tubePresets, "tubes.json");
                }
            });
            tubeButtons.push_back(button);
        }
    }

    QWidget *presetControlsWidget = new QWidget;
    presetControlsWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    presetControlsWidget->setMaximumHeight(400);
    QHBoxLayout *presetControlsLayout = new QHBoxLayout(presetControlsWidget);
    presetControlsLayout->addWidget(tubesWidget);
    presetControlsLayout->addWidget(presetsWidget);

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

    QVBoxLayout* slidersLayout = new QVBoxLayout();
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
    bottomWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    QHBoxLayout *bottomLayout = new QHBoxLayout(bottomWidget);
    bottomWidget->setMaximumHeight(550);
    QGridLayout *gridLayout = new QGridLayout;

    // Loop to create buttons and add them to the layout
    for (int row = 0; row < 10; ++row) {
        for (int col = 0; col < 10; ++col) {
            EffectPresetModel *model = effectPresets[row * 10 + col];
            model->id = row * 10 + col;
            PresetButton *button = new PresetButton(model, this);
            gridLayout->addWidget(button, row, col);
            connect(button, &PresetButton::releasedInstantly, [=](){
                if (activeEffectPresetButton) {
                    activeEffectPresetButton->setStyleSheet("");
                }

                ptrdiff_t index = std::distance(tubeButtons.begin(), std::find(tubeButtons.begin(), tubeButtons.end(), button));
                EffectPresetModel *model = static_cast<EffectPresetModel *>(button->getModel());
                if (currentEffect != -1) {
                    effectButtons[currentEffect]->setStyleSheet("");
                }
                button->setStyleSheet("background-color: red");
                currentEffect = (int)index;
                setNewEffect(model);
                activeEffectPresetButton = button;

                applyTubePreset(model->getPresets());
                sendTubeSyncData();

                for (auto const& [id, preset] : model->getPresets()->getTubePresets()) {
                    std::cout << id << " " << preset.delay << std::endl;
                }
            });
            connect(button, &PresetButton::longPressed, [=](){
                bool ok;
                QString text = QInputDialog::getText(this, tr("Effect config"), tr("Enter effect name:"), QLineEdit::Normal, "", &ok);
                if (ok && !text.isEmpty()) {
                    EffectPresetModel *model = static_cast<EffectPresetModel *>(button->getModel());
                    model->setConfig(ep->getMasterconfig());
                    if (currentPreset != -1) {
                        model->setPresets(*tubePresets[currentPreset]);
                    }
                    model->setName(text.toStdString());
                    button->setModel(model);
                    EffectPresetModel::saveToJsonFile(effectPresets, "effects.json");

                    for (auto const& [id, preset] : model->getPresets()->getTubePresets()) {
                        std::cout << id << " " << preset.delay << std::endl;
                    }
                }
            });
            effectButtons.push_back(button);
        }
    }
    bottomLayout->addLayout(slidersLayout);
    bottomLayout->addLayout(gridLayout);

    QWidget *groupSelectorWidget = new QWidget;
    QHBoxLayout *groupSelectorLayout = new QHBoxLayout(groupSelectorWidget);
    groupSelectorLayout->addWidget(new QLabel("Group Selection:"));
    numBeatLabel = new QLabel("0");
    groupSelectorLayout->addWidget(numBeatLabel);
    for (std::string effect : {"Count Up", "Region", "Random"}) {
        QRadioButton *radio = new QRadioButton();
        radio->setText(effect.c_str());
        connect(radio, &QRadioButton::toggled, this, [=](){

        });
        groupSelectorLayout->addWidget(radio);
    }

    QWidget *colorPaletteWidget = new QWidget;
    QHBoxLayout *colorPaletteLayout = new QHBoxLayout(colorPaletteWidget);

    colorPaletteLayout->addWidget(new QLabel("Color:"));
    for (std::string effect : {"Random", "Red/White", "Custom"}) {
        QRadioButton *radio = new QRadioButton();
        radio->setText(effect.c_str());

        connect(radio, &QRadioButton::toggled, this, [=](){
            if (radio->text() == "Random") {
                qDebug() << "Random";
                colorMode = RandomHue;
            } else {
                colorMode = Palette;
            }
        });
        colorPaletteLayout->addWidget(radio);
    }

    QWidget *headerWidget = new QWidget;
    QHBoxLayout *header = new QHBoxLayout(headerWidget);
    bpmLabel = new QLabel("bpm");
    tmpLabel = new QLabel("name");
    audioCheckBox = new QCheckBox("Audio Filter");
    connect(audioCheckBox, &QCheckBox::toggled, this, [=](bool checked){
        std::cout << checked << std::endl;
        a->setUseFilterOutput(checked);
    });
    header->addWidget(new QLabel("Analysis: "));
    header->addWidget(bpmLabel);
    header->addWidget(tmpLabel);
    header->addWidget(audioCheckBox);

    glv = new OGLWidget(1024);
    glv->setMinimumHeight(100);
    //glv->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    connect(glv, &OGLWidget::threshChanged, this, &AudioWindow::sliderChanged);
    connect(glv, &OGLWidget::rangeChanged, this, [=](){
        a->setFilter(new audiofilter());
        a->getFilter()->setLower(glv->getRegions()[0]->getScaledMin() * (1.0/1024.0) * 24000);
        a->getFilter()->setUpper(glv->getRegions()[0]->getScaledMax() * (1.0/1024.0) * 24000);
    });

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
        QString filename = QFileDialog::getOpenFileName(this, tr("Open Tube Firmware File"), "./", tr("BIN Files (*.bin)"), nullptr, QFileDialog::DontUseNativeDialog);
        if (!filename.isNull()) {
            std::cout << "FLASHING" << std::endl;

            mdnsflasher::flash(filename.toStdString());
        }
    });

    QPushButton *syncbutton = new QPushButton("Sync");
    connect(syncbutton, &QPushButton::pressed, [=](){
        ep->sendSync();
    });

    fwButtons->addWidget(fwbutton);
    fwButtons->addWidget(flashbutton);
    fwButtons->addWidget(syncbutton);

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
    mainLayout->addWidget(presetControlsWidget);
    mainLayout->addWidget(colorPaletteWidget);
    mainLayout->addWidget(groupSelectorWidget);
    mainLayout->addWidget(headerWidget);
    mainLayout->addWidget(glvWidget);
    mainLayout->addWidget(modesWidget);
    mainLayout->addWidget(modifiersWidget);
    mainLayout->addWidget(bottomWidget);
    mainLayout->addWidget(firmwareWidget);

    mainLayout->setStretchFactor(4, 1);

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
    setenv("XDG_DATA_HOME", "/home/josh/.local/share", 1);
    setenv("XDG_CONFIG_HOME", "/home/josh/.config", 1);
    setenv("XDG_CACHE_HOME", "/home/josh/.cache", 1);

    printf("%d\n", getuid());
    printf("%s\n", getenv("USER"));

    a = new AudioAnalyzer();
    a->getdevices();
    a->startRecording();

    QTimer *mainTimer = new QTimer(this);
    connect(mainTimer, &QTimer::timeout, this, &AudioWindow::checkTime);
    mainTimer->start(1);

    QTimer *qtimer = new QTimer(this);
    connect(qtimer, &QTimer::timeout, this, &AudioWindow::checkStatus);
    qtimer->start(500);
}

void AudioWindow::applyTubePreset(const TubePresetModel *model) {
    std::cout << " applyTubePreset " << std::endl;
    for (auto const &[mac, preset] : model->getTubePresets()) {
        std::cout << mac << " " << preset.delay << std::endl;
        for (auto t : tubes) {
            if (t->getMac() == mac) {
                t->blockSignals(true);
                t->setDelay(preset.delay);
                t->setGroup(preset.group);
                t->blockSignals(false);
                continue;
            }
        }
    }
}

void AudioWindow::checkStatus() {
    NetDevice h = NetDevice("wlxdc4ef40a3f9f");

    if (wifiLabel) {
        if (h.checkInterface()) {
            wifiLabel->setText("Online");
        } else {
            wifiLabel->setText("Offline");
        }
    }

}

void AudioWindow::sendTubeSyncData() {
    int max = 1;
    std::vector<int> offsets;
    std::vector<int> groups;
    for (auto t : tubes) {
        offsets.push_back(t->getDelay());
        groups.push_back(t->getGroup());
        max = t->getGroup() > max ? t->getGroup() : max;
    }
    qDebug() << max;
    numGroups = max;
    ep->setTubeGroups(groups);
    ep->setTubeOffsets(offsets);
    ep->sendSyncConfig();
}

void AudioWindow::setNewEffect(EffectPresetModel *model) {
    activeEffect = model->id;
    this->brightnessSlider->setValue(model->config.brightness);
    this->speedSlider->setValue(model->config.speed_factor);
    this->effect1Slider->setValue(model->config.parameter1);
    this->effect2Slider->setValue(model->config.parameter2);
    this->effect3Slider->setValue(model->config.parameter3);
    for (int i = 0; i < 8; ++i) {
        this->ledModifierCheckboxes[i]->blockSignals(true);
        this->ledModifierCheckboxes[i]->setChecked(((model->config.modifiers >> (7 - i)) & 0x01));
        this->ledModifierCheckboxes[i]->blockSignals(false);
    }
    ledModeRadioButtons[model->config.led_mode]->blockSignals(true);
    ledModeRadioButtons[model->config.led_mode]->setChecked(true);
    ledModeRadioButtons[model->config.led_mode]->blockSignals(false);


    if (activeEffectPresetButton) {
        activeEffectPresetButton->setStyleSheet("");
    }
    if (model->id < effectButtons.size()) {
        activeEffectPresetButton = effectButtons[model->id];
        activeEffectPresetButton->setStyleSheet("background-color: red");
    }


    ep->setMasterconfig(model->config);
    ep->sendConfig();


    ptrdiff_t pos = std::distance(tubePresets.begin(), std::find(tubePresets.begin(), tubePresets.end(), model->getPresets()));
    if (pos < tubePresets.size()) {
        currentPreset = (int)pos;
    }

    if (currentPreset != -1) {
        applyTubePreset(tubePresets[currentPreset]);
        sendTubeSyncData();
    }
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

    std::vector<float> out(fl.size());

    if (false) {
        return;
    }

    OGLWidget *w = glv;
    if (fullScreenWindow->isActiveWindow()) {
        w = popoutGlv;
    }

    w->processData(fl, [this](FrequencyRegion &region){
        int tubeGroupValue = 0;
        if (numGroups > 1) {
            tubeGroupValue = (numBeats  % numGroups) + 1;
        }

        if (numBeats % 16 == 0 && autoCheckboxes[1]->isChecked()) {
            setNewEffect(effectPresets[(*effectRandom)(*rng)]);
        }

        numBeats++;
        numBeatLabel->setText(QString::number(tubeGroupValue));
        std::array<float,2> selectedColor = colors.front();
        ep->peakEvent((int)(selectedColor[0] * 255.0), (int)(selectedColor[1] * 255.0), tubeGroupValue);
        std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
        beats.push(region.getBeatMillis());

        for (auto t : tubes) {
            t->setPeaked(hsv2rgb({selectedColor[0] * 360, selectedColor[1], 1.0}), tubeGroupValue);
        }

        if (colorMode == RandomHue) {
            colors.push({(float)(*hueRandom)(*rng) / (float) 360, saturationSlider->pct()});
        } else if (colorMode == RandomColor) {
            colors.push({(float)(*hueRandom)(*rng) / (float) 360, saturationSlider->pct()});
        } else if (colorMode == Palette) {
            if (numGroups > 1) {
                if (lastColorRed) {
                    colors.push({0.0, 1.0});
                    colors.push({0.0, 1.0});
                } else {
                    colors.push({0.0, 0.0});
                    colors.push({0.0, 0.0});
                }
            } else {
                if (lastColorRed) {
                    colors.push({0.0, 1.0});
                } else {
                    colors.push({0.0, 0.0});
                }
            }
            lastColorRed = !lastColorRed;
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
    }  if (sender() == speedSlider) {
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

    for (auto t : tubes) {
        t->setEffect(d);
        t->sync();
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
        for (auto t : tubes) {
            t->setEffect(d);
            t->sync();
        }
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

EffectPresetModel *AudioWindow::getCurrentEffect()
{
    return effectPresets[currentEffect];
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
