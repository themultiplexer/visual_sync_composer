#include "audiowindow.h"
#include "ProjectModel.h"
#include "effectpresetmodel.h"
#include "gltext.h"
#include "wifieventprocessor.h"
#include <boost/circular_buffer.hpp>
#include <QRadioButton>
#include "netdevice.h"
#include "helper.h"

AudioWindow::AudioWindow(WifiEventProcessor *ep, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    this->ep = ep;
    ui->setupUi(this);
    NetDevice h = NetDevice("wlxdc4ef40a3f9f");
    h.setInterface(false);
    h.enableMonitorMode();
    h.setInterface(true);

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
    QHBoxLayout* modesLayout = new QHBoxLayout(centralWidget);

    std::vector<QRadioButton*> ledModeRadioButtons;

    for (std::string effect : values) {
        QRadioButton *radio = new QRadioButton();
        radio->setText(effect.c_str());
        connect(radio, &QRadioButton::toggled, this, &AudioWindow::effectChanged);
        ledModeRadioButtons.push_back(radio);
        modesLayout->addWidget(radio);
    }

    sensitivitySlider = new VSCSlider("Sensitivity", Qt::Horizontal, centralWidget);
    sensitivitySlider->setMinimum(0);
    sensitivitySlider->setValue(50);
    sensitivitySlider->setMaximum(100);

    brightnessSlider = new VSCSlider("Brightness", Qt::Horizontal, centralWidget);
    brightnessSlider->setMinimum(0);
    brightnessSlider->setValue(0);
    brightnessSlider->setMaximum(255);
    connect(brightnessSlider, &VSCSlider::sliderReleased, this, &AudioWindow::sliderChanged);

    speedSlider = new VSCSlider("Speed", Qt::Horizontal, centralWidget);
    speedSlider->setMinimum(1);
    speedSlider->setValue(5);
    speedSlider->setMaximum(255);
    connect(speedSlider, &VSCSlider::sliderReleased, this, &AudioWindow::sliderChanged);

    effect1Slider = new VSCSlider("Effect 1", Qt::Horizontal, centralWidget);
    effect1Slider->setMinimum(1);
    effect1Slider->setValue(5);
    effect1Slider->setMaximum(255);
    connect(effect1Slider, &VSCSlider::sliderReleased, this, &AudioWindow::sliderChanged);

    effect2Slider = new VSCSlider("Effect 2", Qt::Horizontal, centralWidget);
    effect2Slider->setMinimum(1);
    effect2Slider->setValue(5);
    effect2Slider->setMaximum(255);
    connect(effect2Slider, &VSCSlider::sliderReleased, this, &AudioWindow::sliderChanged);

    // Create a layout
    QWidget *gridWidget = new QWidget;
    QGridLayout *gridLayout = new QGridLayout;

    Helper::readJson();
    std::vector<EffectPresetModel> presets = std::vector<EffectPresetModel>();
    for (int i = 0; i < 100; ++i) {
        presets.push_back(EffectPresetModel());
    }

    // Loop to create buttons and add them to the layout
    for (int row = 0; row < 10; ++row) {
        for (int col = 0; col < 10; ++col) {
            EffectPresetButton *button = new EffectPresetButton(QString("Button %1").arg(row * 10 + col + 1), &presets[row * 10 + col + 1]);
            gridLayout->addWidget(button, row, col);
            connect(button, &EffectPresetButton::releasedInstantly, [=](){
                EffectPresetModel *model = button->getModel();
                this->brightnessSlider->setValue(model->config.brightness);
                this->speedSlider->setValue(model->config.speed_factor);
                this->effect1Slider->setValue(model->config.parameter1);
                this->effect2Slider->setValue(model->config.parameter2);
                ledModeRadioButtons[model->config.led_mode]->setEnabled(true);
            });
            connect(button, &EffectPresetButton::longPressed, [this, button](){
                bool ok;
                QString text = QInputDialog::getText(this, tr("QInputDialog::getText()"),
                                                     tr("User name:"), QLineEdit::Normal,
                                                     "", &ok);
                if (ok && !text.isEmpty()) {
                    EffectPresetModel *model = button->getModel();
                    model->config.brightness = this->brightnessSlider->value();
                    model->config.speed_factor = this->speedSlider->value();
                    //model->config.led_mode = ledModeRadioButtons.;
                    model->config.parameter1 = this->effect1Slider->value();
                    model->config.parameter2 = this->effect2Slider->value();
                    button->setText(text);
                    button->setModel(model);

                    //Helper::saveJson();
                }
            });
        }
    }
    gridWidget->setLayout(gridLayout);

    glv = new OGLWidget(centralWidget);
    glv->setMinimumHeight(300);
    //connect(glv, QOpenGLWidget::)

    // Add widgets to the layout
    mainLayout->addWidget(glv);
    mainLayout->addLayout(modesLayout);
    mainLayout->addWidget(sensitivitySlider);
    mainLayout->addWidget(brightnessSlider);
    mainLayout->addWidget(speedSlider);
    mainLayout->addWidget(effect1Slider);
    mainLayout->addWidget(effect2Slider);
    mainLayout->addWidget(gridWidget);

    if (getuid() == 0) {
        printf("Dropping privs\n");
        /* process is running as root, drop privileges */
        if (setgid(1000) != 0)
            printf("setgid: Unable to drop group privileges: %s\n", strerror(errno));
        if (setuid(1000) != 0)
            printf("setuid: Unable to drop user privileges: %s\n", strerror(errno));
    }

    printf("%d\n", getuid());

    a = new AudioAnalyzer();
    a->getdevices();
    a->startRecording();

    QTimer *qtimer = new QTimer(this);
    connect(qtimer, &QTimer::timeout, this, &AudioWindow::checkTime);
    qtimer->start(10);
}

std::chrono::time_point<std::chrono::steady_clock> lastPeak;

void AudioWindow::checkTime(){
    auto f = a->getFullFrequencies();
    auto now = std::chrono::steady_clock::now();

    float lowFreqIntensity = 0.0;
    for (int i = 1; i < 3; ++i) {
        lowFreqIntensity += f[i];
    }

    bool lowpeak = (lowFreqIntensity > sensitivitySlider->value() * 5.0);
    bool debounce = (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastPeak).count() > 100);

    if (lowpeak && debounce) {
        lastPeak = now;
        ep->peakEvent();
    }

    for (int i = 0; i < FRAMES/2; i++) {
        f[i] *= log10(((float)i/(FRAMES/2)) * 5 + 1.01);
    }
    glv->setFrequencies(f, lowpeak && debounce);
}

void AudioWindow::sliderChanged()
{
    if (sender() == brightnessSlider) {
        ep->masterconfig.brightness = brightnessSlider->value();
        ep->sendConfig();
    } else if (sender() == speedSlider) {
        ep->masterconfig.speed_factor = speedSlider->value();
        ep->sendConfig();
    } else if (sender() == effect1Slider) {
        ep->masterconfig.parameter1 = effect1Slider->value();
        ep->sendConfig();
    } else if (sender() == effect2Slider) {
        ep->masterconfig.parameter2 = effect2Slider->value();
        ep->sendConfig();
    }
}

void AudioWindow::effectChanged(bool state)
{
    if(state) {
        QRadioButton* button = qobject_cast<QRadioButton*>(sender());
        auto it = std::find(std::begin(values), std::end(values), button->text().toStdString());
        int index = it - std::begin(values);
        ep->masterconfig.led_mode = index;
        ep->sendConfig();
    }
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
