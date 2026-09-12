#include "core/audiocontroller.h"

#include "RtMidi.h"
#include "core/audioanalyzer.h"
#include "core/audiofilter.h"
#include "core/controllerabstractor.h"
#include "core/devicereqistry.h"
#include "core/effectpresetmodel.h"
#include "core/fixturepresetmodel.h"
#include "core/mdnsflasher.h"
#include "core/midicontroller.h"
#include "core/presetmodel.h"
#include "core/tubepresetmodel.h"
#include "core/wifiadapter.h"
#include "core/wifieventprocessor.h"

#include <QFileInfo>
#include <QVariantMap>

#include <algorithm>
#include <cmath>
#include <map>
#include <numeric>

namespace {

constexpr int effectPresetCount = 100;
constexpr int tubePresetCount = 16;
constexpr int fixturePresetCount = 16;
constexpr int presetsPerBank = 16;
constexpr int spectrumSize = FRAMES / 2;

const QStringList standardEffectNames{
    QStringLiteral("Static"), QStringLiteral("Blink"),
    QStringLiteral("Breath"), QStringLiteral("Color Wipe"),
    QStringLiteral("Color Wipe Inverse"), QStringLiteral("Color Wipe Reverse"),
    QStringLiteral("Color Wipe Reverse Inverse"), QStringLiteral("Color Wipe Random"),
    QStringLiteral("Random Color"), QStringLiteral("Single Dynamic"),
    QStringLiteral("Multi Dynamic"), QStringLiteral("Rainbow"),
    QStringLiteral("Rainbow Cycle"), QStringLiteral("Scan"),
    QStringLiteral("Dual Scan"), QStringLiteral("Fade"),
    QStringLiteral("Theater Chase"), QStringLiteral("Theater Chase Rainbow"),
    QStringLiteral("Running Lights"), QStringLiteral("Twinkle"),
    QStringLiteral("Twinkle Random"), QStringLiteral("Twinkle Fade"),
    QStringLiteral("Twinkle Fade Random"), QStringLiteral("Sparkle"),
    QStringLiteral("Flash Sparkle"), QStringLiteral("Hyper Sparkle"),
    QStringLiteral("Strobe"), QStringLiteral("Strobe Rainbow"),
    QStringLiteral("Multi Strobe"), QStringLiteral("Blink Rainbow"),
    QStringLiteral("Chase White"), QStringLiteral("Chase Color"),
    QStringLiteral("Chase Random"), QStringLiteral("Chase Rainbow"),
    QStringLiteral("Chase Flash"), QStringLiteral("Chase Flash Random"),
    QStringLiteral("Chase Rainbow White"), QStringLiteral("Chase Blackout"),
    QStringLiteral("Chase Blackout Rainbow"), QStringLiteral("Color Sweep Random"),
    QStringLiteral("Running Color"), QStringLiteral("Running Red Blue"),
    QStringLiteral("Running Random"), QStringLiteral("Larson Scanner"),
    QStringLiteral("Comet"), QStringLiteral("Fireworks"),
    QStringLiteral("Fireworks Random"), QStringLiteral("Merry Christmas"),
    QStringLiteral("Fire Flicker"), QStringLiteral("Fire Flicker (soft)"),
    QStringLiteral("Fire Flicker (intense)"), QStringLiteral("Circus Combustus"),
    QStringLiteral("Halloween"), QStringLiteral("Bicolor Chase"),
    QStringLiteral("Tricolor Chase"), QStringLiteral("TwinkleFOX"),
    QStringLiteral("Rain"), QStringLiteral("Block Dissolve"),
    QStringLiteral("ICU"), QStringLiteral("Dual Larson"),
    QStringLiteral("Running Random2"), QStringLiteral("Filler Up"),
    QStringLiteral("Rainbow Larson"), QStringLiteral("Rainbow Fireworks"),
    QStringLiteral("Trifade"), QStringLiteral("VU Meter"),
    QStringLiteral("Heartbeat"), QStringLiteral("Bits"),
    QStringLiteral("Multi Comet"), QStringLiteral("Flipbook"),
    QStringLiteral("Popcorn"), QStringLiteral("Oscillator"),
    QStringLiteral("Custom 0"), QStringLiteral("Custom 1"),
    QStringLiteral("Custom 2"), QStringLiteral("Custom 3"),
    QStringLiteral("Custom 4"), QStringLiteral("Custom 5"),
    QStringLiteral("Custom 6"), QStringLiteral("Custom 7")
};

template<typename T>
T clampEnum(int value, int last)
{
    return static_cast<T>(std::clamp(value, 0, last));
}

QVariantMap presetMap(int id, const std::string &name, const QColor &color)
{
    return {
        {QStringLiteral("id"), id},
        {QStringLiteral("name"), QString::fromStdString(name)},
        {QStringLiteral("color"), color}
    };
}

} // namespace

AudioController::AudioController(WifiEventProcessor *eventProcessor, QObject *parent)
    : QObject(parent)
    , m_eventProcessor(eventProcessor)
    , m_audioAnalyzer(std::make_unique<AudioAnalyzer>())
    , m_midiController(std::make_unique<MidiController>())
    , m_wifiAdapter(std::make_unique<WifiAdapter>("wlxdc4ef40a3f9f"))
    , m_lastEffectChange(std::chrono::steady_clock::now())
    , m_lastPresetChange(std::chrono::steady_clock::now())
{
    Q_ASSERT(m_eventProcessor);

    // ControllerAbstractor should now accept AudioController rather than
    // AudioWindow. It remains a hardware-input adapter, not a view.
    m_controlSurface = std::make_unique<ControllerAbstractor>(this);

    m_audioTimer.setInterval(16);
    m_audioTimer.setTimerType(Qt::PreciseTimer);
    connect(&m_audioTimer, &QTimer::timeout,
            this, &AudioController::processAudioFrame);

    m_statusTimer.setInterval(500);
    connect(&m_statusTimer, &QTimer::timeout,
            this, &AudioController::updateConnectionStatus);

    m_configurationDebounce.setSingleShot(true);
    m_configurationDebounce.setInterval(25);
    connect(&m_configurationDebounce, &QTimer::timeout,
            this, &AudioController::sendPendingConfiguration);

    m_currentPalette = {{
        {0.0F, 1.0F}, {0.0F, 0.0F}, {0.0F, 1.0F},
        {0.0F, 0.0F}, {0.0F, 1.0F}, {0.0F, 0.0F}
    }};

    rebuildEffectNames();
    loadPersistentState();
}

AudioController::~AudioController() = default;

void AudioController::start()
{
    if (m_started)
        return;

    m_started = true;

    m_midiController->start(true);

    m_wifiAdapter->setInterface(false);
    m_wifiAdapter->enableMonitorMode();
    m_wifiAdapter->setInterface(true);

    m_eventProcessor->initHandlers();
    m_audioAnalyzer->getdevices();
    m_audioAnalyzer->startRecording();

    m_audioTimer.start();
    m_statusTimer.start();

    sendHello();
    refreshControlSurface();
    updateConnectionStatus();
}

int AudioController::brightness() const noexcept { return m_brightness; }
int AudioController::speed() const noexcept { return m_speed; }
int AudioController::parameter1() const noexcept { return m_parameter1; }
int AudioController::parameter2() const noexcept { return m_parameter2; }
int AudioController::parameter3() const noexcept { return m_parameter3; }
int AudioController::ledMode() const noexcept { return m_ledMode; }

QVariantList AudioController::modifiers() const
{
    QVariantList result;
    result.reserve(8);
    for (int index = 0; index < 8; ++index)
        result.push_back(bool(m_modifierMask & (1U << (7 - index))));
    return result;
}

int AudioController::groupSelectionMode() const noexcept
{
    return static_cast<int>(m_groupSelection);
}

int AudioController::colorSelectionMode() const noexcept
{
    return static_cast<int>(m_colorSelection);
}

int AudioController::colorControlMode() const noexcept
{
    return static_cast<int>(m_colorControl);
}

QVariantList AudioController::effectPresets() const
{
    QVariantList result;
    result.reserve(static_cast<qsizetype>(m_effectPresetModels.size()));
    for (int index = 0; index < static_cast<int>(m_effectPresetModels.size()); ++index) {
        const auto *preset = m_effectPresetModels[index];
        result.push_back(preset
            ? presetMap(preset->id, preset->getName(), preset->getColor())
            : presetMap(index, {}, {}));
    }
    return result;
}

QVariantList AudioController::tubePresets() const
{
    QVariantList result;
    result.reserve(static_cast<qsizetype>(m_tubePresetModels.size()));
    for (int index = 0; index < static_cast<int>(m_tubePresetModels.size()); ++index) {
        const auto *preset = m_tubePresetModels[index];
        result.push_back(preset
            ? presetMap(preset->id, preset->getName(), preset->getColor())
            : presetMap(index, {}, {}));
    }
    return result;
}

QVariantList AudioController::fixturePresets() const
{
    QVariantList result;
    result.reserve(static_cast<qsizetype>(m_fixturePresetModels.size()));
    for (int index = 0; index < static_cast<int>(m_fixturePresetModels.size()); ++index) {
        const auto *preset = m_fixturePresetModels[index];
        result.push_back(preset
            ? presetMap(preset->id, preset->getName(), preset->getColor())
            : presetMap(index, {}, {}));
    }
    return result;
}

QVariantList AudioController::tubes() const
{
    QVariantList result;
    result.reserve(static_cast<qsizetype>(m_tubeStates.size()));
    for (const TubeState &tube : m_tubeStates) {
        result.push_back(QVariantMap{
            {QStringLiteral("mac"), tube.mac},
            {QStringLiteral("delay"), tube.delay},
            {QStringLiteral("group"), tube.group}
        });
    }
    return result;
}

QVariantList AudioController::palette() const
{
    QVariantList result;
    result.reserve(static_cast<qsizetype>(m_currentPalette.size()));
    for (const HsColor &entry : m_currentPalette) {
        result.push_back(QVariantMap{
            {QStringLiteral("hue"), entry[0]},
            {QStringLiteral("saturation"), entry[1]},
            {QStringLiteral("color"), toDisplayColor(entry)}
        });
    }
    return result;
}

void AudioController::setActiveBank(int bank)
{
    const int bounded = std::clamp(bank, 0, 4);
    if (m_activeBank == bounded)
        return;

    m_activeBank = bounded;
    emit activeBankChanged();

    m_controlSurface->setPage(m_activeBank + 1);
    refreshControlSurface();
}

void AudioController::selectEffectPreset(int index)
{
    if (index < 0 || index >= static_cast<int>(m_effectPresetModels.size()))
        return;

    EffectPresetModel *preset = m_effectPresetModels[index];
    if (!preset)
        return;
    m_activeEffectPreset = index;
    m_activeEffectWithinBank = preset->id % presetsPerBank;

    m_brightness = preset->config.brightness;
    m_speed = preset->config.speed_factor;
    m_parameter1 = preset->config.parameter1;
    m_parameter2 = preset->config.parameter2;
    m_parameter3 = preset->config.parameter3;
    m_ledMode = preset->config.led_mode;
    m_modifierMask = preset->config.modifiers;

    const auto copyCount = std::min(preset->colors.size(), m_currentPalette.size());
    std::copy_n(preset->colors.begin(), copyCount, m_currentPalette.begin());

    CONFIG_DATA configuration = preset->config;
    configuration.brightness = m_brightness;
    configuration.speed_factor = m_speed;
    configuration.parameter1 = m_parameter1;
    configuration.parameter2 = m_parameter2;
    configuration.parameter3 = m_parameter3;
    m_eventProcessor->setMasterconfig(configuration);
    m_eventProcessor->sendConfig(m_activeFixtureGroup);

    std::vector<std::uint8_t> dmxChannels(9);
    for (int channel = 0; channel < 9; ++channel)
        dmxChannels[channel] = preset->dmx_config.channels[channel];
    m_eventProcessor->sendDmx(dmxChannels);

    const int row = m_activeEffectWithinBank / 4;
    const int column = m_activeEffectWithinBank % 4;
    m_controlSurface->updateMatrix();
    m_controlSurface->setMatrixButton(row, column, preset->getColor());
    m_midiController->send(column, row, false);

    emit activeEffectPresetChanged();
    emit effectConfigurationChanged();
    emit paletteChanged();

    if (m_modifierMask & 0x80U)
        triggerPeak();
}

void AudioController::selectTubePreset(int index)
{
    if (index < 0 || index >= static_cast<int>(m_tubePresetModels.size()))
        return;

    const TubePresetModel *presetModel = m_tubePresetModels[index];
    if (!presetModel)
        return;
    for (const auto &[mac, preset] : presetModel->getTubePresets()) {
        const auto tube = std::find_if(m_tubeStates.begin(), m_tubeStates.end(),
            [&mac](const TubeState &candidate) {
                return candidate.mac.toStdString() == mac;
            });
        if (tube != m_tubeStates.end()) {
            tube->delay = preset.delay;
            tube->group = preset.group;
        }
    }

    m_activeTubePreset = index;
    emit activeTubePresetChanged();
    emit tubesChanged();
    synchronizeTubes();
}

void AudioController::selectFixtureGroup(int index)
{
    if (index < 0 || index >= static_cast<int>(m_fixturePresetModels.size())
        || m_activeFixtureGroup == index) {
        return;
    }

    m_activeFixtureGroup = index;
    emit activeFixtureGroupChanged();
}

void AudioController::setBrightness(int value)
{
    value = boundedByte(value);
    if (m_brightness == value)
        return;
    m_brightness = value;
    emit effectConfigurationChanged();
    scheduleConfigurationSend();
}

void AudioController::setSpeed(int value)
{
    value = std::clamp(value, 1, 255);
    if (m_speed == value)
        return;
    m_speed = value;
    emit effectConfigurationChanged();
    scheduleConfigurationSend();
}

void AudioController::setParameter1(int value)
{
    value = std::clamp(value, 1, 255);
    if (m_parameter1 == value)
        return;
    m_parameter1 = value;
    emit effectConfigurationChanged();
    scheduleConfigurationSend();
}

void AudioController::setParameter2(int value)
{
    value = std::clamp(value, 1, 255);
    if (m_parameter2 == value)
        return;
    m_parameter2 = value;
    emit effectConfigurationChanged();
    scheduleConfigurationSend();
}

void AudioController::setParameter3(int value)
{
    value = std::clamp(value, 1, 255);
    if (m_parameter3 == value)
        return;
    m_parameter3 = value;
    emit effectConfigurationChanged();
    scheduleConfigurationSend();
}

void AudioController::setLedMode(int index)
{
    if (m_ledMode == index)
        return;
    m_ledMode = index;
    emit effectConfigurationChanged();
    applyCurrentConfiguration();
}

void AudioController::setModifier(int index, bool enabled)
{
    if (index < 0 || index >= 8)
        return;

    const std::uint8_t bit = std::uint8_t(1U << (7 - index));
    const std::uint8_t changed = enabled
        ? std::uint8_t(m_modifierMask | bit)
        : std::uint8_t(m_modifierMask & ~bit);
    if (changed == m_modifierMask)
        return;

    m_modifierMask = changed;
    emit effectConfigurationChanged();
    applyCurrentConfiguration();
}

void AudioController::setSensitivity(double value)
{
    value = std::clamp(value, 0.0, 1.0);
    if (qFuzzyCompare(m_sensitivity, value))
        return;
    m_sensitivity = value;
    emit sensitivityChanged();
}

void AudioController::setPeakSaturation(double value)
{
    value = std::clamp(value, 0.0, 1.0);
    if (qFuzzyCompare(m_peakSaturation, value))
        return;
    m_peakSaturation = value;
    emit peakSaturationChanged();
}

void AudioController::setSpectrumDecay(double value)
{
    value = std::clamp(value, 0.0, 0.1);
    if (qFuzzyCompare(m_spectrumDecay, value))
        return;
    m_spectrumDecay = value;
    emit spectrumDecayChanged();
}

void AudioController::setInputVolume(double value)
{
    value = std::clamp(value, 0.0, 1.0);
    if (qFuzzyCompare(m_inputVolume, value))
        return;
    m_inputVolume = value;
    m_audioAnalyzer->setInputVolume(m_inputVolume * 100.0);
    emit inputVolumeChanged();
}

void AudioController::setAudioFilterEnabled(bool enabled)
{
    if (m_audioFilterEnabled == enabled)
        return;
    m_audioFilterEnabled = enabled;
    m_audioAnalyzer->setUseFilterOutput(enabled);
    emit audioFilterEnabledChanged();
}

void AudioController::setFilterRange(double normalizedLower,
                                     double normalizedUpper)
{
    normalizedLower = std::clamp(normalizedLower, 0.0, 1.0);
    normalizedUpper = std::clamp(normalizedUpper, 0.0, 1.0);
    if (normalizedLower > normalizedUpper)
        std::swap(normalizedLower, normalizedUpper);

    m_audioAnalyzer->setFilter(new audiofilter());
    m_audioAnalyzer->getFilter()->setLower(normalizedLower * 24000.0);
    m_audioAnalyzer->getFilter()->setUpper(normalizedUpper * 24000.0);
}

void AudioController::setGroupSelectionMode(int mode)
{
    const auto value = clampEnum<GroupSelection>(mode, 2);
    if (m_groupSelection == value)
        return;
    m_groupSelection = value;
    emit groupSelectionModeChanged();
}

void AudioController::setColorSelectionMode(int mode)
{
    const auto value = clampEnum<ColorSelection>(mode, 2);
    if (m_colorSelection == value)
        return;
    m_colorSelection = value;
    emit colorSelectionModeChanged();
}

void AudioController::setColorControlMode(int mode)
{
    const auto value = clampEnum<ColorControl>(mode, 4);
    if (m_colorControl == value)
        return;
    m_colorControl = value;
    emit colorControlModeChanged();
    triggerPeak();
}

void AudioController::selectBuiltInPalette(int index)
{
    static constexpr std::array<Palette, 3> palettes{{
        {{{0.0F, 1.0F}, {0.0F, 0.0F}, {0.0F, 1.0F},
          {0.0F, 0.0F}, {0.0F, 1.0F}, {0.0F, 0.0F}}},
        {{{0.333F, 1.0F}, {0.0F, 0.0F}, {0.333F, 1.0F},
          {0.0F, 0.0F}, {0.333F, 1.0F}, {0.0F, 0.0F}}},
        {{{0.8F, 1.0F}, {0.2F, 1.0F}, {0.8F, 1.0F},
          {0.2F, 1.0F}, {0.8F, 1.0F}, {0.2F, 1.0F}}}
    }};

    if (index < 0 || index >= static_cast<int>(palettes.size()))
        return;

    m_currentPalette = palettes[static_cast<std::size_t>(index)];
    setColorControlMode(static_cast<int>(ColorControl::Palette));
    emit paletteChanged();
}

void AudioController::setPaletteEntry(int index, double hue, double saturation)
{
    if (index < 0 || index >= static_cast<int>(m_currentPalette.size()))
        return;

    m_currentPalette[index] = {
        float(std::clamp(hue, 0.0, 1.0)),
        float(std::clamp(saturation, 0.0, 1.0))
    };
    emit paletteChanged();
}

void AudioController::triggerPeak(int region, int tubeIndex, bool chooseNewColor)
{
    int peakGroup = 0;
    if (m_numGroups > 1) {
        switch (m_groupSelection) {
        case GroupSelection::CountUp:
            peakGroup = (m_numBeats % m_numGroups) + 1;
            break;
        case GroupSelection::Regions:
            peakGroup = region;
            break;
        case GroupSelection::Random:
            peakGroup = std::uniform_int_distribution<int>(1, m_numGroups)(m_randomEngine);
            break;
        }
    }

    if (region >= 0 && region < 4)
        m_controlSurface->flashStopButton(region);

    ++m_numBeats;
    m_beatGroup = peakGroup;
    emit beatStatisticsChanged();

    m_currentColor = choosePeakColor(region, chooseNewColor);
    const int hue = boundedByte(int(m_currentColor[0] * 255.0F));
    const int saturation = boundedByte(int(m_currentColor[1] * 255.0F));
    const QColor displayColor = toDisplayColor(m_currentColor);

    if (tubeIndex >= 0 && tubeIndex < static_cast<int>(m_tubeStates.size())) {
        const auto macs = devicereqistry::macs();
        m_eventProcessor->sendIndividualPeak(macs[tubeIndex], hue, saturation);
        emit tubePeakTriggered(tubeIndex, displayColor, m_activeFixtureGroup);
    } else {
        m_eventProcessor->sendBroadcastPeak(hue, saturation, peakGroup);
        emit allTubesPeakTriggered(displayColor, m_activeFixtureGroup);
    }
}

void AudioController::reportDetectedBeat(int region, double intervalMs)
{
    if (region < 0 || region >= 3 || !m_controlSurface->isCaptureToggled())
        return;

    triggerPeak(region);
    updateBeatStatistics(intervalMs);
}

void AudioController::processAutomaticModes(bool automaticEffects,
                                            bool automaticComposition)
{
    const auto now = std::chrono::steady_clock::now();
    if (automaticEffects) {
        const auto age = std::chrono::duration_cast<std::chrono::seconds>(
            now - m_lastEffectChange);
        if ((m_numBeats > 0 && m_numBeats % 16 == 0) || age.count() >= 40) {
            const int index = m_activeBank * presetsPerBank
                            + m_effectDistribution(m_randomEngine);
            selectEffectPreset(index);
            m_lastEffectChange = now;
            ++m_numBeats;
        }
    }

    if (automaticComposition) {
        const auto age = std::chrono::duration_cast<std::chrono::seconds>(
            now - m_lastPresetChange);
        if ((m_numBeats > 0 && m_numBeats % 16 == 0) || age.count() >= 2) {
            selectTubePreset(m_presetDistribution(m_randomEngine));
            m_lastPresetChange = now;
            ++m_numBeats;
        }
    }
}

void AudioController::updateTube(int index, int delay, int group)
{
    if (index < 0 || index >= static_cast<int>(m_tubeStates.size()))
        return;

    TubeState &tube = m_tubeStates[index];
    delay = boundedByte(delay);
    group = std::max(group, 0);
    if (tube.delay == delay && tube.group == group)
        return;

    tube.delay = delay;
    tube.group = group;
    emit tubesChanged();
    synchronizeTubes();
}

void AudioController::moveTube(int from, int to)
{
    if (from < 0 || to < 0 || from >= static_cast<int>(m_tubeStates.size())
        || to >= static_cast<int>(m_tubeStates.size()) || from == to) {
        return;
    }

    std::swap(m_tubeStates[from], m_tubeStates[to]);
    emit tubesChanged();
    synchronizeTubes();
}

void AudioController::synchronizeTubes()
{
    std::vector<int> offsets;
    std::vector<int> groups;
    offsets.reserve(m_tubeStates.size());
    groups.reserve(m_tubeStates.size());

    m_numGroups = 1;
    for (const TubeState &tube : m_tubeStates) {
        offsets.push_back(tube.delay);
        groups.push_back(tube.group);
        m_numGroups = std::max(m_numGroups, tube.group);
    }

    m_eventProcessor->setTubeGroups(groups);
    m_eventProcessor->setTubeOffsets(offsets);
    m_eventProcessor->sendSyncConfig();
}

void AudioController::saveEffectPreset(int index, const QString &name)
{
    if (index < 0 || index >= static_cast<int>(m_effectPresetModels.size())
        || name.trimmed().isEmpty()) {
        return;
    }

    EffectPresetModel *model = m_effectPresetModels[index];
    if (!model)
        return;
    model->setConfig(m_eventProcessor->getMasterconfig());
    for (int channel = 0; channel < 9; ++channel)
        model->dmx_config.channels[channel] = m_currentDmxData[channel];
    if (m_activeTubePreset >= 0
        && m_activeTubePreset < static_cast<int>(m_tubePresetModels.size())) {
        model->setPresets(*m_tubePresetModels[m_activeTubePreset]);
    }
    model->setName(name.trimmed().toStdString());
    model->colors.assign(m_currentPalette.begin(), m_currentPalette.end());

    PresetModel::saveToJsonFile(m_effectPresetModels, "effects.json");
    emit effectPresetsChanged();
}

void AudioController::moveEffectPreset(int from, int to)
{
    if (from < 0 || to < 0 || from >= static_cast<int>(m_effectPresetModels.size())
        || to >= static_cast<int>(m_effectPresetModels.size()) || from == to
        || !m_effectPresetModels[from] || !m_effectPresetModels[to]) {
        return;
    }

    std::swap(m_effectPresetModels[from], m_effectPresetModels[to]);
    m_effectPresetModels[from]->id = from;
    m_effectPresetModels[to]->id = to;
    PresetModel::saveToJsonFile(m_effectPresetModels, "effects.json");
    emit effectPresetsChanged();
    refreshControlSurface();
}

void AudioController::setEffectPresetColor(int index, const QColor &color)
{
    if (index < 0 || index >= static_cast<int>(m_effectPresetModels.size())
        || !m_effectPresetModels[index] || !color.isValid()) {
        return;
    }
    m_effectPresetModels[index]->color = color;
    PresetModel::saveToJsonFile(m_effectPresetModels, "effects.json");
    emit effectPresetsChanged();
    refreshControlSurface();
}

void AudioController::saveTubePreset(int index, const QString &name)
{
    if (index < 0 || index >= static_cast<int>(m_tubePresetModels.size())
        || name.trimmed().isEmpty()) {
        return;
    }

    std::map<std::string, TubePreset> presets;
    for (const TubeState &tube : m_tubeStates) {
        TubePreset preset;
        preset.delay = tube.delay;
        preset.group = tube.group;
        presets[tube.mac.toStdString()] = preset;
    }

    TubePresetModel *model = m_tubePresetModels[index];
    if (!model)
        return;
    model->setTubePresets(presets);
    model->setName(name.trimmed().toStdString());
    PresetModel::saveToJsonFile(m_tubePresetModels, "tubes.json");
    emit tubePresetsChanged();
}

void AudioController::moveTubePreset(int from, int to)
{
    if (from < 0 || to < 0 || from >= static_cast<int>(m_tubePresetModels.size())
        || to >= static_cast<int>(m_tubePresetModels.size()) || from == to
        || !m_tubePresetModels[from] || !m_tubePresetModels[to]) {
        return;
    }

    std::swap(m_tubePresetModels[from], m_tubePresetModels[to]);
    m_tubePresetModels[from]->id = from;
    m_tubePresetModels[to]->id = to;
    PresetModel::saveToJsonFile(m_tubePresetModels, "tubes.json");
    emit tubePresetsChanged();
}

void AudioController::setTubePresetColor(int index, const QColor &color)
{
    if (index < 0 || index >= static_cast<int>(m_tubePresetModels.size())
        || !m_tubePresetModels[index] || !color.isValid()) {
        return;
    }
    m_tubePresetModels[index]->color = color;
    PresetModel::saveToJsonFile(m_tubePresetModels, "tubes.json");
    emit tubePresetsChanged();
}

void AudioController::sendHello()
{
    m_eventProcessor->sendHelloToAll();
}

void AudioController::putAllTubesIntoFirmwareUpdateMode()
{
    m_eventProcessor->sendUpdateMessage();
}

void AudioController::putTubeIntoFirmwareUpdateMode(int tubeIndex)
{
    const auto macs = devicereqistry::macs();
    if (tubeIndex < 0 || tubeIndex >= static_cast<int>(macs.size()))
        return;
    m_eventProcessor->sendUpdateMessageTo(macs[tubeIndex]);
}

void AudioController::flashFirmware(const QUrl &file)
{
    const QString filename = file.toLocalFile();
    if (filename.isEmpty() || !QFileInfo::exists(filename)) {
        emit errorOccurred(tr("Firmware file does not exist."));
        return;
    }
    mdnsflasher::flash(filename.toStdString());
}

void AudioController::setDmxChannels(const QVariantList &channels)
{
    const qsizetype count = std::min<qsizetype>(channels.size(), 9);
    for (qsizetype index = 0; index < count; ++index)
        m_currentDmxData[index] = std::uint8_t(boundedByte(channels[index].toInt()));
}

void AudioController::processAudioFrame()
{
    m_controlSurface->processEvents();

    auto left = m_audioAnalyzer->getLeftFrequencies();
    auto right = m_audioAnalyzer->getRightFrequencies();

    m_leftSpectrum.clear();
    m_rightSpectrum.clear();
    m_leftSpectrum.reserve(spectrumSize);
    m_rightSpectrum.reserve(spectrumSize);

    for (int index = 0; index < spectrumSize; ++index) {
        const double frequencyWeight = std::log10(
            (double(index) / double(spectrumSize)) * 5.0 + 1.01);
        left[index] = std::log10(left[index] * frequencyWeight * 2.0 + 1.01);
        right[index] = std::log10(right[index] * frequencyWeight * 2.0 + 1.01);
        m_leftSpectrum.push_back(left[index]);
        m_rightSpectrum.push_back(right[index]);
    }

    if (m_colorControl == ColorControl::Frequency && spectrumSize > 0) {
        const auto maximum = std::max_element(left.begin(), left.end());
        const auto index = std::distance(left.begin(), maximum);
        m_currentPalette[0] = {
            float(index) / float(spectrumSize) * 3.5F,
            1.0F
        };
        emit paletteChanged();
    }

    detectSpectrumBeats();
    emit spectrumChanged();
}

void AudioController::updateConnectionStatus()
{
    const bool current = m_wifiAdapter->checkInterface();
    if (current == m_online)
        return;
    m_online = current;
    emit onlineChanged();
}

void AudioController::sendPendingConfiguration()
{
    applyCurrentConfiguration();
}

void AudioController::loadPersistentState()
{
    m_effectPresetModels =
        PresetModel::readJson<EffectPresetModel, effectPresetCount>("effects.json");
    for (int index = 0; index < static_cast<int>(m_effectPresetModels.size()); ++index)
        m_effectPresetModels[index]->id = index;

    m_tubePresetModels.resize(tubePresetCount);
    const auto storedTubePresets =
        PresetModel::readJson<TubePresetModel, tubePresetCount>("tubes.json");
    for (TubePresetModel *preset : storedTubePresets) {
        if (preset->index >= 0 && preset->index < tubePresetCount)
            m_tubePresetModels[preset->index] = preset;
    }
    for (int index = 0; index < static_cast<int>(m_tubePresetModels.size()); ++index) {
        if (m_tubePresetModels[index])
            m_tubePresetModels[index]->id = index;
    }

    m_fixturePresetModels =
        PresetModel::readJson<FixturePresetModel, fixturePresetCount>("fixtures.json");
    for (int index = 0; index < static_cast<int>(m_fixturePresetModels.size()); ++index)
        m_fixturePresetModels[index]->id = index;

    for (const auto &mac : devicereqistry::macs())
        m_tubeStates.push_back({QString::fromStdString(arrayToHexString(mac)), 0, 0});
}

void AudioController::rebuildEffectNames()
{
    m_effectNames = standardEffectNames;
}

void AudioController::applyCurrentConfiguration()
{
    CONFIG_DATA configuration = m_eventProcessor->getMasterconfig();
    configuration.brightness = m_brightness;
    configuration.speed_factor = m_speed;
    configuration.parameter1 = m_parameter1;
    configuration.parameter2 = m_parameter2;
    configuration.parameter3 = m_parameter3;
    configuration.led_mode = m_ledMode;
    configuration.modifiers = m_modifierMask;

    m_eventProcessor->setMasterconfig(configuration);
    m_eventProcessor->sendConfig(m_activeFixtureGroup);
}

void AudioController::scheduleConfigurationSend()
{
    m_configurationDebounce.start();
}

void AudioController::refreshControlSurface()
{
    m_controlSurface->updateMatrix();
}

void AudioController::detectSpectrumBeats()
{
    if (m_leftSpectrum.isEmpty() || !m_controlSurface->isCaptureToggled())
        return;

    static constexpr std::array<double, 4> regionLimits{0.0, 0.08, 0.25, 1.0};
    const auto now = std::chrono::steady_clock::now();

    for (int region = 0; region < 3; ++region) {
        const qsizetype begin = qsizetype(
            regionLimits[region] * double(m_leftSpectrum.size()));
        const qsizetype end = std::max<qsizetype>(
            begin + 1,
            qsizetype(regionLimits[region + 1] * double(m_leftSpectrum.size())));

        double level = 0.0;
        for (qsizetype index = begin;
             index < std::min(end, m_leftSpectrum.size()); ++index) {
            level = std::max(level, m_leftSpectrum[index].toDouble());
        }

        m_regionEnvelopes[region] = std::max(
            level, m_regionEnvelopes[region] - m_spectrumDecay);
        const bool above = m_regionEnvelopes[region] >= m_sensitivity;

        const auto sinceLast = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - m_lastRegionBeat[region]);
        if (above && !m_regionAboveThreshold[region]
            && (m_lastRegionBeat[region].time_since_epoch().count() == 0
                || sinceLast.count() > 120)) {
            const double interval = m_lastRegionBeat[region].time_since_epoch().count() == 0
                ? 0.0 : double(sinceLast.count());
            m_lastRegionBeat[region] = now;
            triggerPeak(region);
            updateBeatStatistics(interval);
        }

        m_regionAboveThreshold[region] = above;
    }
}

void AudioController::updateBeatStatistics(double intervalMs)
{
    if (!std::isfinite(intervalMs) || intervalMs <= 0.0)
        return;

    constexpr std::size_t maximumSamples = 32;
    m_recentBeatIntervals.push_back(intervalMs);
    if (m_recentBeatIntervals.size() > maximumSamples)
        m_recentBeatIntervals.erase(m_recentBeatIntervals.begin());

    m_meanBeatIntervalMs = std::accumulate(m_recentBeatIntervals.begin(),
                                           m_recentBeatIntervals.end(), 0.0)
                         / double(m_recentBeatIntervals.size());
    m_bpm = m_meanBeatIntervalMs > 0.0
        ? int(std::lround(60000.0 / m_meanBeatIntervalMs))
        : 0;
    emit beatStatisticsChanged();
}

AudioController::HsColor AudioController::choosePeakColor(int region,
                                                          bool chooseNewColor)
{
    if (!chooseNewColor)
        return m_currentColor;

    switch (m_colorControl) {
    case ColorControl::Frequency:
    case ColorControl::Manual:
        return m_currentPalette[0];
    case ColorControl::RandomHue:
        return {float(m_hueDistribution(m_randomEngine)) / 360.0F,
                float(m_peakSaturation)};
    case ColorControl::RandomColor:
        return {float(m_hueDistribution(m_randomEngine)) / 360.0F,
                float(m_byteDistribution(m_randomEngine)) / 255.0F};
    case ColorControl::Palette:
        switch (m_colorSelection) {
        case ColorSelection::CountUp: {
            const HsColor color = m_currentPalette[m_paletteIndex];
            m_paletteIndex = (m_paletteIndex + 1) % 4;
            return color;
        }
        case ColorSelection::Regions:
            return m_currentPalette[std::clamp(region > 0 ? region - 1 : 0, 0, 5)];
        case ColorSelection::Random:
            return m_currentPalette[m_paletteDistribution(m_randomEngine)];
        }
    }
    return m_currentColor;
}

QColor AudioController::toDisplayColor(const HsColor &color) const
{
    return QColor::fromHsvF(std::clamp(double(color[0]), 0.0, 1.0),
                            std::clamp(double(color[1]), 0.0, 1.0), 1.0);
}

int AudioController::boundedByte(int value)
{
    return std::clamp(value, 0, 255);
}
