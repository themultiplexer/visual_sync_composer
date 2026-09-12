#pragma once

#include <QObject>
#include <QColor>
#include <QElapsedTimer>
#include <QStringList>
#include <QTimer>
#include <QUrl>
#include <QVariantList>

#include <array>
#include <chrono>
#include <cstdint>
#include <memory>
#include <random>
#include <vector>

class AudioAnalyzer;
class ControllerAbstractor;
class EffectPresetModel;
class FixturePresetModel;
class MidiController;
class TubePresetModel;
class WifiAdapter;
class WifiEventProcessor;

// AudioController contains application state and commands only. It deliberately
// has no dependency on QWidget, QQuickItem, QQmlContext, or any visual object.
class AudioController final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool online READ online NOTIFY onlineChanged)
    Q_PROPERTY(int beatGroup READ beatGroup NOTIFY beatStatisticsChanged)
    Q_PROPERTY(int bpm READ bpm NOTIFY beatStatisticsChanged)
    Q_PROPERTY(double meanBeatIntervalMs READ meanBeatIntervalMs NOTIFY beatStatisticsChanged)

    Q_PROPERTY(int activeBank READ activeBank WRITE setActiveBank NOTIFY activeBankChanged)
    Q_PROPERTY(int activeEffectPreset READ activeEffectPreset NOTIFY activeEffectPresetChanged)
    Q_PROPERTY(int activeTubePreset READ activeTubePreset NOTIFY activeTubePresetChanged)
    Q_PROPERTY(int activeFixtureGroup READ activeFixtureGroup NOTIFY activeFixtureGroupChanged)

    Q_PROPERTY(int brightness READ brightness WRITE setBrightness NOTIFY effectConfigurationChanged)
    Q_PROPERTY(int speed READ speed WRITE setSpeed NOTIFY effectConfigurationChanged)
    Q_PROPERTY(int parameter1 READ parameter1 WRITE setParameter1 NOTIFY effectConfigurationChanged)
    Q_PROPERTY(int parameter2 READ parameter2 WRITE setParameter2 NOTIFY effectConfigurationChanged)
    Q_PROPERTY(int parameter3 READ parameter3 WRITE setParameter3 NOTIFY effectConfigurationChanged)
    Q_PROPERTY(int ledMode READ ledMode WRITE setLedMode NOTIFY effectConfigurationChanged)
    Q_PROPERTY(QVariantList modifiers READ modifiers NOTIFY effectConfigurationChanged)

    Q_PROPERTY(double sensitivity READ sensitivity WRITE setSensitivity NOTIFY sensitivityChanged)
    Q_PROPERTY(double peakSaturation READ peakSaturation WRITE setPeakSaturation NOTIFY peakSaturationChanged)
    Q_PROPERTY(double spectrumDecay READ spectrumDecay WRITE setSpectrumDecay NOTIFY spectrumDecayChanged)
    Q_PROPERTY(double inputVolume READ inputVolume WRITE setInputVolume NOTIFY inputVolumeChanged)
    Q_PROPERTY(bool audioFilterEnabled READ audioFilterEnabled WRITE setAudioFilterEnabled NOTIFY audioFilterEnabledChanged)

    Q_PROPERTY(int groupSelectionMode READ groupSelectionMode WRITE setGroupSelectionMode NOTIFY groupSelectionModeChanged)
    Q_PROPERTY(int colorSelectionMode READ colorSelectionMode WRITE setColorSelectionMode NOTIFY colorSelectionModeChanged)
    Q_PROPERTY(int colorControlMode READ colorControlMode WRITE setColorControlMode NOTIFY colorControlModeChanged)

    Q_PROPERTY(QStringList effectNames READ effectNames CONSTANT)
    Q_PROPERTY(QVariantList effectPresets READ effectPresets NOTIFY effectPresetsChanged)
    Q_PROPERTY(QVariantList tubePresets READ tubePresets NOTIFY tubePresetsChanged)
    Q_PROPERTY(QVariantList fixturePresets READ fixturePresets NOTIFY fixturePresetsChanged)
    Q_PROPERTY(QVariantList tubes READ tubes NOTIFY tubesChanged)
    Q_PROPERTY(QVariantList palette READ palette NOTIFY paletteChanged)
    Q_PROPERTY(QVariantList leftSpectrum READ leftSpectrum NOTIFY spectrumChanged)
    Q_PROPERTY(QVariantList rightSpectrum READ rightSpectrum NOTIFY spectrumChanged)

public:
    enum class GroupSelection {
        CountUp,
        Regions,
        Random
    };
    Q_ENUM(GroupSelection)

    enum class ColorSelection {
        CountUp,
        Regions,
        Random
    };
    Q_ENUM(ColorSelection)

    enum class ColorControl {
        Frequency,
        Manual,
        RandomHue,
        RandomColor,
        Palette
    };
    Q_ENUM(ColorControl)

    explicit AudioController(WifiEventProcessor *eventProcessor,
                             QObject *parent = nullptr);
    ~AudioController() override;

    bool online() const noexcept { return m_online; }
    int beatGroup() const noexcept { return m_beatGroup; }
    int bpm() const noexcept { return m_bpm; }
    double meanBeatIntervalMs() const noexcept { return m_meanBeatIntervalMs; }

    int activeBank() const noexcept { return m_activeBank; }
    int activeEffectPreset() const noexcept { return m_activeEffectPreset; }
    int activeTubePreset() const noexcept { return m_activeTubePreset; }
    int activeFixtureGroup() const noexcept { return m_activeFixtureGroup; }

    int brightness() const noexcept;
    int speed() const noexcept;
    int parameter1() const noexcept;
    int parameter2() const noexcept;
    int parameter3() const noexcept;
    int ledMode() const noexcept;
    QVariantList modifiers() const;

    double sensitivity() const noexcept { return m_sensitivity; }
    double peakSaturation() const noexcept { return m_peakSaturation; }
    double spectrumDecay() const noexcept { return m_spectrumDecay; }
    double inputVolume() const noexcept { return m_inputVolume; }
    bool audioFilterEnabled() const noexcept { return m_audioFilterEnabled; }

    int groupSelectionMode() const noexcept;
    int colorSelectionMode() const noexcept;
    int colorControlMode() const noexcept;

    const QStringList &effectNames() const noexcept { return m_effectNames; }
    QVariantList effectPresets() const;
    QVariantList tubePresets() const;
    QVariantList fixturePresets() const;
    QVariantList tubes() const;
    QVariantList palette() const;
    const QVariantList &leftSpectrum() const noexcept { return m_leftSpectrum; }
    const QVariantList &rightSpectrum() const noexcept { return m_rightSpectrum; }

public slots:
    void start();

    void setActiveBank(int bank);
    void selectEffectPreset(int index);
    void selectTubePreset(int index);
    void selectFixtureGroup(int index);

    void setBrightness(int value);
    void setSpeed(int value);
    void setParameter1(int value);
    void setParameter2(int value);
    void setParameter3(int value);
    void setLedMode(int index);
    Q_INVOKABLE void setModifier(int index, bool enabled);

    void setSensitivity(double value);
    void setPeakSaturation(double value);
    void setSpectrumDecay(double value);
    void setInputVolume(double value);
    void setAudioFilterEnabled(bool enabled);
    Q_INVOKABLE void setFilterRange(double normalizedLower, double normalizedUpper);

    void setGroupSelectionMode(int mode);
    void setColorSelectionMode(int mode);
    void setColorControlMode(int mode);
    Q_INVOKABLE void selectBuiltInPalette(int index);
    Q_INVOKABLE void setPaletteEntry(int index, double hue, double saturation);

    Q_INVOKABLE void triggerPeak(int region = 0,
                                 int tubeIndex = -1,
                                 bool chooseNewColor = true);
    Q_INVOKABLE void reportDetectedBeat(int region, double intervalMs);
    Q_INVOKABLE void processAutomaticModes(bool automaticEffects,
                                           bool automaticComposition);

    Q_INVOKABLE void updateTube(int index, int delay, int group);
    Q_INVOKABLE void moveTube(int from, int to);
    Q_INVOKABLE void synchronizeTubes();

    Q_INVOKABLE void saveEffectPreset(int index, const QString &name);
    Q_INVOKABLE void moveEffectPreset(int from, int to);
    Q_INVOKABLE void setEffectPresetColor(int index, const QColor &color);
    Q_INVOKABLE void saveTubePreset(int index, const QString &name);
    Q_INVOKABLE void moveTubePreset(int from, int to);
    Q_INVOKABLE void setTubePresetColor(int index, const QColor &color);

    Q_INVOKABLE void sendHello();
    Q_INVOKABLE void putAllTubesIntoFirmwareUpdateMode();
    Q_INVOKABLE void putTubeIntoFirmwareUpdateMode(int tubeIndex);
    Q_INVOKABLE void flashFirmware(const QUrl &file);
    Q_INVOKABLE void setDmxChannels(const QVariantList &channels);

signals:
    void onlineChanged();
    void beatStatisticsChanged();
    void activeBankChanged();
    void activeEffectPresetChanged();
    void activeTubePresetChanged();
    void activeFixtureGroupChanged();
    void effectConfigurationChanged();
    void sensitivityChanged();
    void peakSaturationChanged();
    void spectrumDecayChanged();
    void inputVolumeChanged();
    void audioFilterEnabledChanged();
    void groupSelectionModeChanged();
    void colorSelectionModeChanged();
    void colorControlModeChanged();
    void effectPresetsChanged();
    void tubePresetsChanged();
    void fixturePresetsChanged();
    void tubesChanged();
    void paletteChanged();
    void spectrumChanged();

    // Presentation reacts to these events. The controller never calls a visual
    // object directly.
    void tubePeakTriggered(int tubeIndex, const QColor &color, int group);
    void allTubesPeakTriggered(const QColor &color, int group);
    void errorOccurred(const QString &message);

private slots:
    void processAudioFrame();
    void updateConnectionStatus();
    void sendPendingConfiguration();

private:
    struct TubeState {
        QString mac;
        int delay = 0;
        int group = 0;
    };

    using HsColor = std::array<float, 2>;
    using Palette = std::array<HsColor, 6>;

    void loadPersistentState();
    void rebuildEffectNames();
    void applyCurrentConfiguration();
    void scheduleConfigurationSend();
    void refreshControlSurface();
    void detectSpectrumBeats();
    void updateBeatStatistics(double intervalMs);
    HsColor choosePeakColor(int region, bool chooseNewColor);
    QColor toDisplayColor(const HsColor &color) const;
    static int boundedByte(int value);

    WifiEventProcessor *m_eventProcessor = nullptr; // non-owning
    std::unique_ptr<AudioAnalyzer> m_audioAnalyzer;
    std::unique_ptr<MidiController> m_midiController;
    std::unique_ptr<ControllerAbstractor> m_controlSurface;
    std::unique_ptr<WifiAdapter> m_wifiAdapter;

    std::vector<EffectPresetModel *> m_effectPresetModels;
    std::vector<TubePresetModel *> m_tubePresetModels;
    std::vector<FixturePresetModel *> m_fixturePresetModels;
    std::vector<TubeState> m_tubeStates;

    QStringList m_effectNames;
    QVariantList m_leftSpectrum;
    QVariantList m_rightSpectrum;

    bool m_started = false;
    bool m_online = false;
    bool m_audioFilterEnabled = false;
    int m_activeBank = 0;
    int m_activeEffectPreset = 0;
    int m_activeTubePreset = 0;
    int m_activeFixtureGroup = 0;
    int m_activeEffectWithinBank = 0;
    int m_numBeats = 0;
    int m_numGroups = 1;
    int m_beatGroup = 0;
    int m_bpm = 0;
    double m_meanBeatIntervalMs = 0.0;
    double m_sensitivity = 0.8;
    double m_peakSaturation = 0.5;
    double m_spectrumDecay = 0.05;
    double m_inputVolume = 0.5;
    std::uint8_t m_modifierMask = 0;
    int m_brightness = 0;
    int m_speed = 5;
    int m_parameter1 = 5;
    int m_parameter2 = 5;
    int m_parameter3 = 128;
    int m_ledMode = 0;

    GroupSelection m_groupSelection = GroupSelection::Regions;
    ColorSelection m_colorSelection = ColorSelection::Regions;
    ColorControl m_colorControl = ColorControl::Palette;
    Palette m_currentPalette{};
    HsColor m_currentColor{0.0F, 1.0F};
    int m_paletteIndex = 0;

    std::array<std::uint8_t, 9> m_currentDmxData{};
    std::vector<double> m_recentBeatIntervals;
    std::array<double, 3> m_regionEnvelopes{};
    std::array<bool, 3> m_regionAboveThreshold{};
    std::array<std::chrono::steady_clock::time_point, 3> m_lastRegionBeat{};
    std::mt19937 m_randomEngine{std::random_device{}()};
    std::uniform_int_distribution<int> m_hueDistribution{0, 360};
    std::uniform_int_distribution<int> m_byteDistribution{0, 255};
    std::uniform_int_distribution<int> m_effectDistribution{0, 15};
    std::uniform_int_distribution<int> m_presetDistribution{0, 15};
    std::uniform_int_distribution<int> m_paletteDistribution{0, 5};

    std::chrono::steady_clock::time_point m_lastEffectChange;
    std::chrono::steady_clock::time_point m_lastPresetChange;
    QTimer m_audioTimer;
    QTimer m_statusTimer;
    QTimer m_configurationDebounce;
};
