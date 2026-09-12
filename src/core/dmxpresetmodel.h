#ifndef DMXPRESETMODEL_H
#define DMXPRESETMODEL_H
#include "wifieventprocessor.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <map>
#include "presetmodel.h"

class DmxPreset {
public:
    int delay;
    int group;
    DMX_DATA pattern;
};

class DmxPresetModel : public PresetModel
{
public:
    DmxPresetModel(std::string name, int index, int id = 0);
    DmxPresetModel(std::string name, int index, int id, std::map<std::string, DmxPreset> presets, QColor color);

    int index, id;
    struct PATTERN_DATA pattern;
    std::map<std::string, DmxPreset> tubePresets;

    QJsonObject toJson() const;
    static DmxPresetModel * fromJson(const QJsonObject &obj);

    std::map<std::string, DmxPreset> getTubePresets() const;
    void setTubePresets(const std::map<std::string, DmxPreset> &newPresets);
};

#endif // DMXPRESETMODEL_H
