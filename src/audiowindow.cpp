#include "audiowindow.h"
#include "audiofilter.h"
#include "fullscreenwindow.h"
#include "mdnsflasher.h"
#include "devicereqistry.h"
#include <QDockWidget>

#define USE_DOCK 0

AudioWindow::AudioWindow(WifiEventProcessor *ep, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), popoutGlv(nullptr), fullScreenWindow(new FullscreenWindow()), currentEffect(nullptr), currentPreset(nullptr)
{
    this->ep = ep;
    ui->setupUi(this);
    //NetDevice h = NetDevice("wlxdc4ef40a3f9f");
    //h.setInterface(false);
    //h.enableMonitorMode();
    //h.setInterface(true);
    lastColorRed = false;

    effectPresets = PresetModel::readJson<EffectPresetModel>("effects.json");
    tubePresets = PresetModel::readJson<TubePresetModel>("tubes.json");

    ep->initHandlers();
    //showFullScreen();

    rng = new std::mt19937(dev());
    hueRandom = new std::uniform_int_distribution<std::mt19937::result_type>(0, 360);

    for (auto color : colors) {
        hsv col = {0.0, 1.0, 0.0};
        colors.push(col);
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

    QSplitter* mainLayout = new QSplitter(Qt::Vertical, this);
    this->setCentralWidget(mainLayout);
    mainLayout->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);
    //splitter->setStretchFactor(0, 0);

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
    for (std::string effect : {"Fadeout After Peak","2","3","4","5","6","7","8"}) {
        QCheckBox *check = new QCheckBox();
        check->setText(effect.c_str());
        connect(check, &QCheckBox::toggled, this, &AudioWindow::modifierChanged);
        ledModifierCheckboxes.push_back(check);
        modifiersLayout->addWidget(check);
    }
    modifiersLayout->addStretch();

    QWidget *tubesWidget = new QWidget;
    //tubesWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    QHBoxLayout* tubesLayout = new QHBoxLayout(tubesWidget);
    auto macs = devicereqistry::macs();
    for (auto mac : macs) {
        VSCTube *tube = new VSCTube(arrayToHexString(mac), this);
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

    QWidget *presetsWidget = new QWidget;
    //tubesWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    QGridLayout* presetsLayout = new QGridLayout(presetsWidget);
    for (int row = 0; row < 5; ++row) {
        for (int col = 0; col < 5; ++col) {
            TubePresetModel *model = tubePresets[row * 10 + col];
            PresetButton *button = new PresetButton(model, this);
            presetsLayout->addWidget(button, row, col);
            connect(button, &PresetButton::releasedInstantly, [=](){
                TubePresetModel *model = static_cast<TubePresetModel *>(button->getModel());
                if (currentPreset) {
                    tubeButtons.at(currentPreset)->setStyleSheet("");
                }
                button->setStyleSheet("background-color: red");
                std::cout << "WTF" << std::endl;
                for (auto const &[mac, preset] : model->getTubePresets()) {
                    std::cout << mac << " " << preset.delay << std::endl;
                    for (auto t : tubes) {
                        if (t->getMac() == mac) {
                            t->setValue(preset.delay);
                            continue;
                        }
                    }
                }
                currentPreset = model;

            });
            connect(button, &PresetButton::longPressed, [=](){
                bool ok;
                QString text = QInputDialog::getText(this, tr("Tube Config"), tr("Enter preset name:"), QLineEdit::Normal, "", &ok);
                if (ok && !text.isEmpty()) {

                    std::map<std::string, TubePreset> preset;
                    for (auto t : tubes) {
                        TubePreset p = TubePreset();
                        p.delay = t->value();
                        preset[t->getMac()] = p;
                    }

                    TubePresetModel *model = static_cast<TubePresetModel *>(button->getModel());
                    model->setTubePresets(preset);
                    model->setName(text.toStdString());
                    button->setModel(model);
                    PresetModel::saveToJsonFile(tubePresets, "tubes.json");
                }
            });
            tubeButtons[model] = button;
        }
    }

    QWidget *presetControlsWidget = new QWidget;
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
    bottomWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);
    QHBoxLayout *bottomLayout = new QHBoxLayout(bottomWidget);
    QGridLayout *gridLayout = new QGridLayout;

    // Loop to create buttons and add them to the layout
    for (int row = 0; row < 10; ++row) {
        for (int col = 0; col < 10; ++col) {
            EffectPresetModel *model = effectPresets[row * 10 + col];
            PresetButton *button = new PresetButton(model, this);
            gridLayout->addWidget(button, row, col);
            connect(button, &PresetButton::releasedInstantly, [=](){
                EffectPresetModel *model = static_cast<EffectPresetModel *>(button->getModel());
                if (currentEffect) {
                    effectButtons.at(currentEffect)->setStyleSheet("");
                }
                button->setStyleSheet("background-color: red");
                currentEffect = model;
                setNewEffect(model);
            });
            connect(button, &PresetButton::longPressed, [=](){
                bool ok;
                QString text = QInputDialog::getText(this, tr("Effect config"), tr("Enter effect name:"), QLineEdit::Normal, "", &ok);
                if (ok && !text.isEmpty()) {
                    EffectPresetModel *model = static_cast<EffectPresetModel *>(button->getModel());
                    model->config = ep->getMasterconfig();
                    model->setName(text.toStdString());
                    button->setModel(model);
                    EffectPresetModel::saveToJsonFile(effectPresets, "effects.json");
                }
            });
            effectButtons[model] = button;
        }
    }
    bottomLayout->addLayout(slidersLayout);
    bottomLayout->addLayout(gridLayout);

    QWidget *groupSelectorWidget = new QWidget;
    QHBoxLayout *groupSelectorLayout = new QHBoxLayout(groupSelectorWidget);
    groupSelectorLayout->addWidget(new QLabel("Group Selection:"));
    for (std::string effect : {"Count Up", "Region", "Random"}) {
        QRadioButton *radio = new QRadioButton();
        radio->setText(effect.c_str());
        connect(radio, &QRadioButton::toggled, this, [=](){

        });
        groupSelectorLayout->addWidget(radio);
    }

    QWidget *colorPaletteWidget = new QWidget;
    QHBoxLayout *colorPaletteLayout = new QHBoxLayout(colorPaletteWidget);
    QPushButton *allButton = new QPushButton("Random", colorPaletteWidget);
    QPushButton *testButton = new QPushButton("Red/White", colorPaletteWidget);

    for (auto button : {allButton, testButton}) {
        QObject::connect(button, &QPushButton::released, [=](){
            if (button == testButton) {
                qDebug() << "Pressed";
                colorMode = Palette;
            } else {
                colorMode = RandomHue;
            }
        });
    }
    colorPaletteLayout->addWidget(new QLabel("Color:"));
    colorPaletteLayout->addWidget(allButton);
    colorPaletteLayout->addWidget(testButton);

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
    mainLayout->addWidget(presetControlsWidget);
    mainLayout->addWidget(colorPaletteWidget);
    mainLayout->addWidget(groupSelectorWidget);
    mainLayout->addWidget(headerWidget);
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

    ep->setMasterconfig(model->config);
    ep->sendConfig();
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

    OGLWidget *w = glv;
    if (fullScreenWindow->isActiveWindow()) {
        w = popoutGlv;
    }

    w->processData(fl, [this](FrequencyRegion &region){
        if (region.getScaledMax() < 1000) {
            hsv selectedColor = colors.front();
            ep->peakEvent((int)(selectedColor.h * 255.0), (int)(selectedColor.s * 255.0));
            std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
            beats.push(region.getBeatMillis());

            for (auto t : tubes) {
                t->setPeaked(hsv2rgb(selectedColor));
            }

            if (colorMode == RandomHue) {
                hsv col = {static_cast<double>((float)(*hueRandom)(*rng) / (float) 360), saturationSlider->pct(), 0.0};
                colors.push(col);
            } else if (colorMode == RandomHue) {
                hsv col = {static_cast<double>((float)(*hueRandom)(*rng) / (float) 360), saturationSlider->pct(), 0.0};
                colors.push(col);
            } else if (colorMode == Palette) {
                if (lastColorRed) {
                    colors.push({0.0, 1.0, 0.0});
                } else {
                    colors.push({0.0, 0.0, 0.0});
                }
                lastColorRed = !lastColorRed;
            }

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

EffectPresetModel *AudioWindow::getCurrentEffect() const
{
    return currentEffect;
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
