#pragma once

#include <QString>
#include <QString>
#include <QJsonObject>
#include <QList>
#include <algorithm>

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QList>
#include <QString>

#include <stdexcept>
#include <vector>

struct DmxRegion {
    int start = 0;
    int end = 255;

    QString type;
    QString name;

    QJsonObject properties;

    bool contains(int value) const {
        return value >= start && value <= end;
    }
};


class DmxSlider {
public:
    explicit DmxSlider(QString name = {}, int channel = 0)
        : m_name(std::move(name)),
          m_channel(channel)
    {}

    const QString& name() const {
        return m_name;
    }

    int channel() const {
        return m_channel;
    }

    int value() const {
        return m_value;
    }

    void setValue(int value) {
        m_value = std::clamp(value, 0, 255);
    }

    void addRegion(DmxRegion region) {
        m_regions.append(std::move(region));
    }

    const QList<DmxRegion>& regions() const {
        return m_regions;
    }

    const DmxRegion* activeRegion() const {
        return regionAt(m_value);
    }

    const DmxRegion* regionAt(int value) const {
        for (const auto& region : m_regions) {
            if (region.contains(value))
                return &region;
        }

        return nullptr;
    }

private:
    QString m_name;
    int m_channel = 0;
    int m_value = 0;

    QList<DmxRegion> m_regions;
};

struct DmxFixture {
    QString path;
    QString name;
    QString mode;
    QList<DmxSlider> sliders;
};

struct DmxManufacturer {
    QString key;
    QString name;
};

static std::vector<DmxManufacturer> loadManufacturers(const QString& path)
{
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly))
        throw std::runtime_error("Cannot open manufacturer file");

    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(file.readAll(), &error);

    if (error.error != QJsonParseError::NoError || !doc.isObject())
        throw std::runtime_error("Invalid fixture JSON");

    const auto root = doc.object();

    std::vector<DmxManufacturer> manufacturers;

    for (auto key : root.keys()) {
        manufacturers.push_back({key, root.value(key)["name"].toString()});
    }

    return manufacturers;
}

static DmxFixture loadFixture(const QString& path, const QString& modeName = "")
{
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly))
        throw std::runtime_error("Cannot open fixture file");

    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(file.readAll(), &error);

    if (error.error != QJsonParseError::NoError || !doc.isObject())
        throw std::runtime_error("Invalid fixture JSON");

    const auto root = doc.object();

    DmxFixture fixture;
    fixture.path = path;
    fixture.name = root.value("name").toString();

    // Find requested mode
    QJsonObject selectedMode;

    for (const auto& modeValue : root.value("modes").toArray()) {
        const auto mode = modeValue.toObject();

        if (mode.value("name").toString() == modeName || mode.value("shortName").toString() == modeName) {
            selectedMode = mode;
            break;
        }
    }

    if (selectedMode.isEmpty()) {
        selectedMode = root.value("modes")[0].toObject();
        qDebug() << "Fixture mode not found";
    }

    fixture.mode = selectedMode.value("name").toString();

    const auto availableChannels =
        root.value("availableChannels").toObject();

    const auto channels =
        selectedMode.value("channels").toArray();

    // Mode defines DMX channel order
    for (int i = 0; i < channels.size(); ++i) {
        const auto channelName = channels[i].toString();

        // Null entries represent unused DMX slots
        if (channelName.isEmpty())
            continue;

        const auto channel =
            availableChannels.value(channelName).toObject();

        if (channel.isEmpty())
            continue;

        DmxSlider slider(channelName, i);

        QJsonArray capabilities;

        if (channel.contains("capabilities")) {
            capabilities =
                channel.value("capabilities").toArray();
        } else if (channel.contains("capability")) {
            // Single capability without an explicit range
            QJsonObject capability =
                channel.value("capability").toObject();
            capability.insert("dmxRange",
                              QJsonArray{0, 255});
            capabilities.append(capability);
        }

        for (const auto& capValue : capabilities) {
            const auto cap = capValue.toObject();
            const auto range = cap.value("dmxRange").toArray();

            if (range.size() != 2)
                continue;

            DmxRegion region;
            region.start = range[0].toInt();
            region.end = range[1].toInt();
            region.type = cap.value("type").toString();

            // Optional human-readable name
            region.name = cap.value("comment").toString();

            if (region.name.isEmpty())
                region.name = region.type;

            region.properties = cap;
            slider.addRegion(region);
        }

        fixture.sliders.append(slider);
    }

    return fixture;
}
