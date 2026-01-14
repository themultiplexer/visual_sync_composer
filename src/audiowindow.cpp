#include "audiowindow.h"
#include "audiofilter.h"
#include "dmxwindow.h"
#include "fullscreenwindow.h"
#include "knobwidget.h"
#include "mdnsflasher.h"
#include "devicereqistry.h"
#include "radioselection.h"

#include <QDockWidget>

#define USE_DOCK 0

void AudioWindow::sendToMidiController(int index) {
    //receiver->send(lastButton % 4, lastButton / 4, false);
    lastButton = index;
    //receiver->send(index % 4, index / 4, true);
}

void AudioWindow::sendSliderChanged(int index, int value) {
        std::cout << "Slider Changed" << index << std::endl;
        std::vector<VSCSlider *> sliders = { brightnessSlider, speedSlider, effect1Slider, effect2Slider};

        if (sliders[index]->getIsInverted()) {
            sliders[index]->setValue(255 - value);
        } else {
            sliders[index]->setValue(value);
        }
        sliderDidChanged = true;
}

void AudioWindow::sendKnobChanged(int index, int value) {
        std::cout << "Knob Changed " << value << std::endl;

        knobWidgets[index]->setOuterPercentage((float)value/225.0f);
        knobChanged = true;
}

void AudioWindow::sendButtonPress(int index) {
    qDebug() << "pressed!";
    if (index < 4) {
        peakEvent(index % 2 + 1);
        buttonAfterglow[index]= 1.0;
    } else {
        controller->setSpecialButton(SpecialLEDButton::SHIFT, 1.0);
    }
}

void AudioWindow::sendButtonRelease(int index) {
    qDebug() << "released!";
    if (index < 4) {

    } else {
        controller->setSpecialButton(SpecialLEDButton::SHIFT, 0.0);
    }
}

void AudioWindow::sendMatrixButtonPress(int col, int row) {
    int button = (col * 4) + row;
    std::cout << "Button Pressed" << button << std::endl;
    if (button < 16) {
        setNewEffect(button);
        controller->setMatrixButton(col, row, LEDColor::white, 1.0);
    } else {

    }
}
void AudioWindow::sendMatrixButtonRelease(int col, int row) {
    int button = (col * 4) + row;
    std::cout << "Button Released" << button << std::endl;
    if (button < 16) {
        sendToMidiController(button);
        controller->setMatrixButton(col, row, (LEDColor)(button+1), 0.8);
    } else {

    }
}



AudioWindow::AudioWindow(WifiEventProcessor *ep, QWidget *parent)
    : QMainWindow(parent)
    , popoutGlv(nullptr), activeEffectPresetButton(nullptr), activeTubePresetButton(nullptr), fullScreenWindow(new FullscreenWindow()), wifiLabel(nullptr), currentEffect(0), currentPreset(0), currentTab(0), timer(nullptr), tubeFrames(0), tubePresets(16), currentPaletteIndex(0), receiver(nullptr)
{
    numBeats = 0;
    numGroups = 1;

    groupMode = GroupSelection::CountUp;

    //colorPalette = {{0.0, 1.0}, {1.0, 1.0}};

    this->ep = ep;
    NetDevice h = NetDevice("wlxdc4ef40a3f9f");
    h.setInterface(false);
    h.enableMonitorMode();
    h.setInterface(true);
    lastColorRed = false;

    effectPresets = PresetModel::readJson<EffectPresetModel, 100>("effects.json");
    auto tubeConfigs = PresetModel::readJson<TubePresetModel, 16>("tubes.json");
    for (auto tube : tubeConfigs) {
        tubePresets[tube->index] = tube;
    }

    ep->initHandlers();

    rng = new std::mt19937(dev());
    hueRandom = new std::uniform_int_distribution<std::mt19937::result_type>(0, 360);
    satRandom = new std::uniform_int_distribution<std::mt19937::result_type>(0, 255);
    effectRandom = new std::uniform_int_distribution<std::mt19937::result_type>(0, 15);
    presetRandom = new std::uniform_int_distribution<std::mt19937::result_type>(0, 15);
    paletteRandom = new std::uniform_int_distribution<std::mt19937::result_type>(0, 6);


    lastEffectChange = std::chrono::system_clock::now();
    lastPresetChange = std::chrono::system_clock::now();

    lastDmxSent = std::chrono::system_clock::now();

    for (auto color : colors) {
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
    QVBoxLayout *superLayout = new QVBoxLayout(superWidget);
    this->setCentralWidget(superWidget);

    QSplitter* mainLayout = new QSplitter(Qt::Vertical, superWidget);
    superLayout->addWidget(mainLayout);

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

    QWidget *statusWidget = new QWidget(superWidget);
    QHBoxLayout* statusLayout = new QHBoxLayout(statusWidget);
    statusLayout->setContentsMargins(0,0,0,0);
    statusLayout->addWidget(new QLabel("Status:"));
    wifiLabel = new QLabel("Offline");
    statusLayout->addWidget(wifiLabel);

    QPushButton *fwbutton = new QPushButton("Put All in Firmwareupdate Mode");
    connect(fwbutton, &QPushButton::pressed, [=](){
        ep->sendUpdateMessage();
    });

    QPushButton *flashbutton = new QPushButton("FW Update");
    connect(flashbutton, &QPushButton::pressed, [=, this](){
        QString filename = QFileDialog::getOpenFileName(this, tr("Open Tube Firmware File"), "./", tr("BIN Files (*.bin)"), nullptr, QFileDialog::DontUseNativeDialog);
        if (!filename.isNull()) {
            std::cout << "FLASHING" << std::endl;

            mdnsflasher::flash(filename.toStdString());
        }
    });

    QPushButton *syncbutton = new QPushButton("Send Hello");
    connect(syncbutton, &QPushButton::pressed, [=](){
        ep->sendHelloToAll();
    });

    QPushButton *dmxbutton = new QPushButton("Dmx Tool");
    connect(dmxbutton, &QPushButton::pressed, [=](){
        DmxWindow *dmxw = new DmxWindow(ep);
        dmxw->show();
    });



    statusLayout->addWidget(fwbutton);
    statusLayout->addWidget(flashbutton);
    statusLayout->addWidget(syncbutton);
    statusLayout->addWidget(dmxbutton);

    QWidget *effectSettingsWidget = new QWidget;
    QVBoxLayout *effectSettingsLayout = new QVBoxLayout(effectSettingsWidget);

    QGroupBox *modesWidget = new QGroupBox(effectSettingsWidget);
    modesWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    QHBoxLayout* modesLayout = new QHBoxLayout(modesWidget);

    for (std::string effect : values) {
        QRadioButton *radio = new QRadioButton();
        radio->setText(effect.c_str());
        connect(radio, &QRadioButton::toggled, this, &AudioWindow::effectChanged);
        ledModeRadioButtons.push_back(radio);
        modesLayout->addWidget(radio);
    }
    modesWidget->setMaximumHeight(75);
    modesWidget->setLayout(modesLayout);

    QHBoxLayout* modifiersLayout = new QHBoxLayout(effectSettingsWidget);
    modifiersLayout->addWidget(new QLabel("Modifiers:"));
    for (std::string effect : {"Fadeout After Peak","No Color Delay","Reversed","Stickiness","Pseudo Random","Sync On Peak","7","8"}) {
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
    //modifiersLayout->setContentsMargins(0,0,0,0);
    effectSettingsLayout->setContentsMargins(0,0,0,0);
    effectSettingsLayout->setSpacing(0);
    effectSettingsLayout->addWidget(modesWidget);
    effectSettingsLayout->addLayout(modifiersLayout);

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

        connect(tube, &VSCTube::flashClicked, this, [=, this](){
            auto it = std::find(tubes.begin(), tubes.end(), tube);
            if (it != tubes.end()) {
                int index = std::distance(tubes.begin(), it);
                ep->sendUpdateMessageTo(macs[index]);
            }
        });

        connect(tube, &VSCTube::buttonPressed, this, [=, this](bool right){
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
    connect(brightnessSlider, &VSCSlider::valueChanged, this, &AudioWindow::sliderChanged);

    speedSlider = new VSCSlider("Speed", Qt::Horizontal, tubesWidget, true);
    speedSlider->setMinimum(1);
    speedSlider->setValue(5);
    speedSlider->setMaximum(255);
    connect(speedSlider, &VSCSlider::valueChanged, this, &AudioWindow::sliderChanged);

    effect1Slider = new VSCSlider("Param 1", Qt::Horizontal, tubesWidget);
    effect1Slider->setMinimum(1);
    effect1Slider->setValue(5);
    effect1Slider->setMaximum(255);
    connect(effect1Slider, &VSCSlider::valueChanged, this, &AudioWindow::sliderChanged);

    effect2Slider = new VSCSlider("Param 2", Qt::Horizontal, tubesWidget);
    effect2Slider->setMinimum(1);
    effect2Slider->setValue(5);
    effect2Slider->setMaximum(255);
    connect(effect2Slider, &VSCSlider::valueChanged, this, &AudioWindow::sliderChanged);

    effect3Slider = new VSCSlider("Param 3", Qt::Horizontal, tubesWidget);
    effect3Slider->setMinimum(1);
    effect3Slider->setValue(128);
    effect3Slider->setMaximum(255);
    connect(effect3Slider, &VSCSlider::valueChanged, this, &AudioWindow::sliderChanged);

    effect4Slider = new VSCSlider("Param 4", Qt::Horizontal, tubesWidget);
    effect4Slider->setMinimum(1);
    effect4Slider->setValue(128);
    effect4Slider->setMaximum(255);
    connect(effect4Slider, &VSCSlider::valueChanged, this, &AudioWindow::sliderChanged);

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
    QHBoxLayout *bottomLayout = new QHBoxLayout(bottomWidget);

    QTabWidget *tabWidget = new QTabWidget(bottomWidget);

    tabWidget->setMinimumWidth(600);
    connect(tabWidget, &QTabWidget::currentChanged, [=, this](int index){
        std::cout << index << std::endl;
        currentTab = index;
    });

    for (int tab = 0; tab < 4; ++tab) {
        // Loop to create buttons and add them to the layout
        QWidget *gridWidget = new QWidget;
        gridWidget->setMaximumWidth(550);
        gridWidget->setMaximumHeight(550);
        QHBoxLayout *tabArea = new QHBoxLayout(gridWidget);
        QGridLayout *gridLayout = new QGridLayout;
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                int index = tab * 16 + row * 4 + col;

                EffectPresetModel *model = effectPresets[index];
                model->id = index;
                PresetButton *button = new PresetButton(model, this);
                button->setMinimumWidth(120);
                button->setMaximumWidth(120);
                button->setMinimumHeight(120);
                button->setMaximumHeight(120);
                gridLayout->addWidget(button, row, col);
                connect(button, &PresetButton::releasedInstantly, [=, this](){
                    ptrdiff_t index = std::distance(tubeButtons.begin(), std::find(tubeButtons.begin(), tubeButtons.end(), button));
                    EffectPresetModel *model = static_cast<EffectPresetModel *>(button->getModel());
                    button->setActive(true);
                    currentEffect = (int)index;
                    setNewEffect(model);
                    sendToMidiController((row * 4) + col);

                    autoCheckboxes[1]->setChecked(false);

                    for (auto const& [id, preset] : model->getPresets()->getTubePresets()) {
                        std::cout << id << " " << preset.delay << std::endl;
                    }
                    peakEvent();
                });
                connect(button, &PresetButton::leftLongPressed, [=, this](){
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
                connect(button, &PresetButton::rightLongPressed, [=, this](){
                    bool ok;
                    int newindex = QInputDialog::getInt(this, tr("Effect position"), tr("Enter index:"), -1, 0, 64, 1, &ok);
                    if (newindex != -1 && ok) {
                        EffectPresetModel *oldmodel = static_cast<EffectPresetModel *>(effectButtons[newindex]->getModel());
                        EffectPresetModel *newmodel = static_cast<EffectPresetModel *>(button->getModel());

                        ptrdiff_t index = std::distance(effectButtons.begin(), std::find(effectButtons.begin(), effectButtons.end(), button));

                        oldmodel->id = index;
                        button->setModel(oldmodel);

                        newmodel->id = newindex;
                        effectButtons[newindex]->setModel(newmodel);

                        EffectPresetModel::saveToJsonFile(effectPresets, "effects.json");
                    }
                });
                effectButtons.push_back(button);
            }
        }

        tabArea->addLayout(gridLayout);
        tabArea->addStretch();

        tabWidget->addTab(gridWidget, tr(("Bank " + std::to_string(tab)).c_str()));
    }

    QWidget *presetsWidget = new QWidget;
    QGridLayout* presetsLayout = new QGridLayout(presetsWidget);
    presetsWidget->setMaximumWidth(550);
    presetsWidget->setMaximumHeight(550);
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            int index = row * 4 + col;
            TubePresetModel *model = tubePresets[index];
            model->id = index;
            PresetButton *button = new PresetButton(model, this);
            button->setMinimumWidth(120);
            button->setMaximumWidth(120);
            button->setMinimumHeight(120);
            button->setMaximumHeight(120);
            presetsLayout->addWidget(button, row, col);
            connect(button, &PresetButton::releasedInstantly, [=, this](){
                ptrdiff_t index = std::distance(tubeButtons.begin(), std::find(tubeButtons.begin(), tubeButtons.end(), button));
                TubePresetModel *model = static_cast<TubePresetModel *>(button->getModel());
                if (currentPreset != -1) {
                    tubeButtons[currentPreset]->setActive(false);
                }
                button->setActive(true);
                std::cout << "WTF" << std::endl;
                applyTubePreset(model);
                currentPreset = (int)index;
                activeTubePresetButton = button;
                sendTubeSyncData();

                autoCheckboxes[2]->setChecked(false);

                for (auto t : tubes) {
                    t->sync();
                }
            });
            connect(button, &PresetButton::leftLongPressed, [=, this](){
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
            connect(button, &PresetButton::rightLongPressed, [=, this](){
                bool ok;
                int newindex = QInputDialog::getInt(this, tr("Preset position"), tr("Enter index:"), -1, 0, 16, 1, &ok);
                if (newindex != -1 && ok) {
                    TubePresetModel *oldmodel = static_cast<TubePresetModel *>(tubeButtons[newindex]->getModel());
                    TubePresetModel *newmodel = static_cast<TubePresetModel *>(button->getModel());

                    ptrdiff_t index = std::distance(tubeButtons.begin(), std::find(tubeButtons.begin(), tubeButtons.end(), button));

                    oldmodel->id = index;
                    button->setModel(oldmodel);

                    newmodel->id = newindex;
                    tubeButtons[newindex]->setModel(newmodel);

                    TubePresetModel::saveToJsonFile(tubePresets, "tubes.json");
                }
            });
            tubeButtons.push_back(button);
        }
    }

    QPushButton *button = new QPushButton(bottomWidget);
    button->setMinimumWidth(100);
    button->setMaximumWidth(100);
    button->setMinimumHeight(100);
    button->setMaximumHeight(100);
    connect(button, &QPushButton::pressed, [=, this](){
        peakEvent();
    });

    QVBoxLayout *layout = new QVBoxLayout(bottomWidget);

    QHBoxLayout* knobLayout = new QHBoxLayout();
    for (int i = 0; i < 4; ++i) {
        KnobWidget *knob = new KnobWidget();
        knob->setMinimumHeight(150);
        knob->setMaximumHeight(150);
        knob->setMinimumWidth(150);
        knob->setMaximumWidth(150);
        connect(knob, &KnobWidget::verticalMouseMovement, this, [=, this](float diff){
            if (shiftPressed) {
                knob->setInnerPercentage(std::clamp(knob->getInnerPercentage() - diff, 0.0f, 1.0f));
            } else {
                knob->setOuterPercentage(std::clamp(knob->getOuterPercentage() - diff, 0.0f, 1.0f));
            }

            knobChanged = true;
        });

        knobWidgets.push_back(knob);
        knobLayout->addWidget(knob);
    }
    knobLayout->setContentsMargins(0,0,0,0);
    layout->addLayout(knobLayout);
    layout->addWidget(presetsWidget);
    knobLayout->setSpacing(10);

    bottomLayout->addLayout(slidersLayout);
    bottomLayout->addWidget(tabWidget);
    bottomLayout->addLayout(layout);
    bottomLayout->addWidget(button);
    bottomLayout->setStretchFactor(slidersLayout, 1);


    glv = new OGLWidget(1024);
    glv->setMinimumHeight(100);
    connect(glv, &OGLWidget::threshChanged, this, &AudioWindow::sliderChanged);
    connect(glv, &OGLWidget::rangeChanged, this, [=, this](){
        a->setFilter(new audiofilter());
        a->getFilter()->setLower(glv->getRegions()[0]->getScaledMin() * (1.0/1024.0) * 24000);
        a->getFilter()->setUpper(glv->getRegions()[0]->getScaledMax() * (1.0/1024.0) * 24000);
    });

    numBeatLabel = new QLabel("0");


    QWidget *topWidget = new QWidget;
    QVBoxLayout *topLayout = new QVBoxLayout(topWidget);
    topLayout->setSpacing(0);

    RadioSelection *groupSelection = new RadioSelection("Group Selection Mode:", {"Count Up", "Region", "Random"}, [=, this](int i){
        groupMode = (GroupSelection)i;
    }, 0, this);
    topLayout->addWidget(groupSelection);

    RadioSelection *colorSelection = new RadioSelection("Color Selection Mode:", {"Count Up", "Region", "Random"}, [=, this](int i){
        colorSelectionMode = (ColorSelectionMode)i;
    }, 1, this);
    topLayout->addWidget(colorSelection);

    RadioSelection *visualModeSelection = new RadioSelection("FFT Vis Mode:", {"Exp Mean", "Mean", "Variance"}, [=, this](int i){
        glv->setVisMode((VisMode)i);
    }, 0, this);
    topLayout->addWidget(visualModeSelection);


    std::vector<std::array<std::array<float, 2>, 6>> palettes;
    palettes.push_back({{ {0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f} }});
    palettes.push_back({{ {0.333f, 1.0f}, {0.0f, 0.0f}, {0.333f, 1.0f}, {0.0f, 0.0f}, {0.333f, 1.0f}, {0.0f, 0.0f} }});
    palettes.push_back({{ {0.8f, 1.0f}, {0.2f, 1.0f}, {0.8f, 1.0f}, {0.2f, 1.0f}, {0.8f, 1.0f}, {0.2f, 1.0f} }});

    RadioSelection *paletteSelection = new RadioSelection("Color Palettes:", {"Manual", "Random Hue", "Random Hue&Sat", "Red|White", "Greenish",  "Custom"}, [=, this](int i){
        if (i == 0) {
            colorMode = ColorControl::Manual;
        } else if (i == 1) {
            colorMode = ColorControl::RandomHue;
        } else if (i == 2) {
            colorMode = ColorControl::RandomColor;
        } else {
            colorMode = ColorControl::Palette;
            currentPalette = palettes[i - 3];
            for (int i = 0; i < knobWidgets.size(); ++i) {
                rgb color = hsv2rgb({currentPalette[i][0] * 360.0f, currentPalette[i][1], 1.0});
                knobWidgets[i]->setOuterPercentage(currentPalette[i][0]);
                knobWidgets[i]->setColor(QColor(color.r * 255, color.g * 255, color.b * 255));
            }
        }
        peakEvent();
    }, 3, this);
    topLayout->addWidget(paletteSelection);


    QHBoxLayout *header = new QHBoxLayout(topWidget);

    header->addWidget(numBeatLabel);
    bpmLabel = new QLabel("bpm");
    tmpLabel = new QLabel("name");
    audioCheckBox = new QCheckBox("Audio Filter");
    connect(audioCheckBox, &QCheckBox::toggled, this, [=, this](bool checked){
        std::cout << checked << std::endl;
        a->setUseFilterOutput(checked);
    });
    header->addWidget(new QLabel("Analysis: "));
    header->addWidget(bpmLabel);
    header->addWidget(tmpLabel);
    header->addWidget(audioCheckBox);
    topLayout->addLayout(header);

    // Create a dock widget to hold it
    if (USE_DOCK) {
        dock = new QDockWidget("OpenGL View", this);
        dock->setWidget(glv);
        dock->setAllowedAreas(Qt::TopDockWidgetArea);
    }

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
    mainLayout->addWidget(statusWidget);
    mainLayout->addWidget(tubesWidget);
    mainLayout->addWidget(topWidget);
    mainLayout->addWidget(glvWidget);
    mainLayout->addWidget(effectSettingsWidget);
    mainLayout->addWidget(bottomWidget);

    //mainLayout->setStretchFactor(4, 1);

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


    superWidget->setMaximumHeight(2400);
    mainLayout->resize(mainLayout->minimumSizeHint());
    superWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    this->installEventFilter(this);
    setFocusPolicy(Qt::StrongFocus);

    //t1.join();
    ep->sendHelloToAll();

    std::cout << "WTF" << std::endl;

    controller = new ControllerHandler();
    controller->setDelegate(this);

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            controller->setMatrixButton(i, j, (LEDColor)(i*4+j+1), 0.8);
        }
    }
}


bool AudioWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == this) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Shift) {
                qDebug() << "Shift pressed";
                shiftPressed = true;
                return true;
            }
        } else if (event->type() == QEvent::KeyRelease) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Shift) {
                shiftPressed = false;
                qDebug() << "Shift released";
                return true;
            }
        }
    }
    // Default processing
    return QWidget::eventFilter(obj, event);
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

    if (activeTubePresetButton) {
        activeTubePresetButton->setActive(false);
    }
    if (model->id < tubeButtons.size()) {
        activeTubePresetButton = tubeButtons[model->id];
        activeTubePresetButton->setActive(true);
    }

    for (auto t : tubes) {
        t->sync();
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

void AudioWindow::setNewEffect(int index) {
    setNewEffect(effectPresets[index]);
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
        activeEffectPresetButton->setActive(false);
    }
    if (model->id < effectButtons.size()) {
        activeEffectPresetButton = effectButtons[model->id];
        activeEffectPresetButton->setActive(true);
    }

    CONFIG_DATA d = slidersToConfig(model->config);
    ep->setMasterconfig(d);
    ep->sendConfig();

    applyTubePreset(model->getPresets());
    sendTubeSyncData();

    /*
    ptrdiff_t pos = std::distance(tubePresets.begin(), std::find(tubePresets.begin(), tubePresets.end(), model->getPresets()));
    if (pos < tubePresets.size()) {
        currentPreset = (int)pos;
    }

    if (currentPreset != -1) {
        applyTubePreset(tubePresets[currentPreset]);
        sendTubeSyncData();
    }*/
}

float test;

void AudioWindow::peakEvent(int group) {
    int currentGroup = 0;
    if (groupMode == GroupSelection::CountUp) {
        if (numGroups > 1) {
            currentGroup = (numBeats  % numGroups) + 1;
        }
    } else if (groupMode == GroupSelection::Regions) {
        currentGroup = group;
    }

    int button = 15 + group;

    if (group < 4) {
        buttonAfterglow[group] = 1.0;
    }

    numBeats++;
    numBeatLabel->setText(QString::number(currentGroup));

    // TODO Generating a random palette would be cooler.
    std::array<float, 2> color;
    if (colorMode == ColorControl::Manual) {
        currentColor = currentPalette[0];
        qDebug() << currentColor[0] << " " << currentColor[1];
    } else if (colorMode == ColorControl::RandomHue) {
        currentColor = {(float)(*hueRandom)(*rng) / (float) 360, saturationSlider->pct()};
    } else if (colorMode == ColorControl::RandomColor) {
        currentColor = {(float)(*hueRandom)(*rng) / (float) 360, (float)(*satRandom)(*rng) / (float) 255};
    } else if (colorMode == ColorControl::Palette) {
        if (colorSelectionMode == ColorSelectionMode::CountUp) {
            currentColor = currentPalette[currentPaletteIndex];
            currentPaletteIndex++;
            currentPaletteIndex %= 6;
        } else if (colorSelectionMode == ColorSelectionMode::Random) {
            currentColor = currentPalette[(*paletteRandom)(*rng)];
        } else if (colorSelectionMode == ColorSelectionMode::Regions) {
            currentColor = currentPalette[group > 0 ? group - 1 : 0];
        }
    }
    int hue = currentColor[0] * 255.0;
    int sat = currentColor[1] * 255.0;

    ep->peakEvent(hue, sat, currentGroup);

    rgb c = hsv2rgb({(hue/255.0) * 360.0, sat/255.0, 1.0});
    ep->sendDmx({c.r, c.g, c.b, 0.0, 1.0});

    for (auto t : tubes) {
        t->setPeaked(hsv2rgb({currentColor[0] * 360, currentColor[1], 1.0}), currentGroup);
    }
}

void AudioWindow::checkTime(){
    controller->run();

    for (int i = 0; i < 4; i++) {
        if (buttonAfterglow[i] > 0.0) {
            buttonAfterglow[i] -= 0.05;
            
            controller->setStopButton(i, buttonAfterglow[i]);
        }
    }

    auto fl = a->getLeftFrequencies();
    auto fr = a->getRightFrequencies();

    for (int i = 0; i < FRAMES/2; i++) {
        fl[i] *= log10(((float)i/(FRAMES/2)) * 5 + 1.01);
        fl[i] = log10(fl[i] * 2.0 + 1.01);

        fr[i] *= log10(((float)i/(FRAMES/2)) * 5 + 1.01);
        fr[i] = log10(fr[i] * 2.0 + 1.01);
    }

    int freq_index = std::distance(std::begin(fl), std::max_element(std::begin(fl), std::end(fl)));
    float log_freq_index = ((float)freq_index / (float)1024);
    currentPalette[0] = {log_freq_index * 3.5f, 1.0};

    if (knobChanged) {
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastKnobChange).count();
        if (diff >= 10) {
            for (int i = 0; i < 4; ++i) {
                currentPalette[i][0] = knobWidgets[i]->getOuterPercentage();
                currentPalette[i][1] = knobWidgets[i]->getInnerPercentage();
            }
            peakEvent();
            knobChanged = false;
            lastKnobChange = std::chrono::system_clock::now();
        }
    }
    if (sliderDidChanged) {
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastSliderChange).count();
        if (diff >= 20) {
            CONFIG_DATA d = slidersToConfig(ep->getMasterconfig());
            ep->setMasterconfig(d);
            ep->sendConfigTo({0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF});
            lastSliderChange = std::chrono::system_clock::now();
            sliderDidChanged = false;
        }
    }

    if (false) {
        return;
    }

    OGLWidget *w = glv;
    if (fullScreenWindow->isActiveWindow()) {
        w = popoutGlv;
    }

    if (autoCheckboxes[1]->isChecked()) {
        auto diff = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - lastEffectChange).count();
        if (numBeats % 16 == 0 || diff >= 2) {
            int index = (*effectRandom)(*rng);
            currentEffect = currentTab * 16 + index;
            setNewEffect(effectPresets[currentEffect]);
            sendToMidiController(index);
            lastEffectChange = std::chrono::system_clock::now();
            numBeats++;
        }
    }

    if (autoCheckboxes[2]->isChecked()) {
        auto diff = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - lastPresetChange).count();
        if (numBeats % 16 == 0 || diff >= 2) {
            currentPreset = (*presetRandom)(*rng);
            applyTubePreset(tubePresets[currentPreset]);
            sendTubeSyncData();
            lastPresetChange = std::chrono::system_clock::now();
            numBeats++;
        }
    }

    w->setFrequencies(fl, fr);
    w->processData([this](FrequencyRegion &region){
        int i = region.getIndex();

        if (i >= 3) {
            return;
        }

        peakEvent(i);
        beats.push(region.getBeatMillis());

        float sum = std::accumulate(beats.begin(), beats.end(), 0.0);
        float mean = sum / beats.size();
        bpmLabel->setText(std::to_string((int)std::round(60.0 / (mean / 1000.0))).c_str());
        tmpLabel->setText(std::to_string(mean).c_str());
    });

    for (auto t : tubes) {
        t->updateGL();
    }

    /* TODO think about plain DMX mode...
    auto now = std::chrono::system_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - lastDmxSent).count();
    if (diff > 10) {
        ep->sendDmx((int)(currentColor[0] * 255.0), (int)(currentColor[1] * 255.0), 255, 0);
        brightness = brightness > 0 ? brightness - 1 : 0;
        lastDmxSent = now;
    }
    */
}

CONFIG_DATA AudioWindow::slidersToConfig(CONFIG_DATA d) {
    d.brightness = brightnessSlider->value();
    d.speed_factor = speedSlider->value();
    d.parameter1 = effect1Slider->value();
    d.parameter2 = effect2Slider->value();
    d.parameter3 = effect3Slider->value();
    return d;
}

void AudioWindow::sliderChanged()
{
    if (sender() == glv) {
        sensitivitySlider->setValue(glv->getThresh() * 100);
    } else if (sender() == timeSlider) {
        glv->setDecay(timeSlider->pct() * 0.1);
    } else if (sender() == sensitivitySlider) {
        glv->setThresh(sensitivitySlider->pct());
    } else {        
        sliderDidChanged = true;
    }
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

    for (auto t : tubes) {
        t->setEffect(d);
        t->sync();
    }

    ep->setMasterconfig(d);
    ep->sendConfig();
}

EffectPresetModel *AudioWindow::getCurrentEffect()
{
    return effectPresets[currentEffect];
}

void AudioWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton resBtn = QMessageBox::question( this, "Quittin'?",
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
    receiver->setDone(true);
}
