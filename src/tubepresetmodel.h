#ifndef TUBEPRESETMODEL_H
#define TUBEPRESETMODEL_H
#include "wifieventprocessor.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <map>
#include "presetmodel.h"

class TubePreset {
public:
    int delay;
    int group;
};

class TubePresetModel : public PresetModel
{
public:
    TubePresetModel(std::string name, int id = 0);
    TubePresetModel(std::string name, int id, std::map<std::string, TubePreset> presets);


    int id;
    std::map<std::string, TubePreset> tubePresets;

    QJsonObject toJson() const;
    static TubePresetModel * fromJson(const QJsonObject &obj);

    std::map<std::string, TubePreset> getTubePresets() const;
    void setTubePresets(const std::map<std::string, TubePreset> &newTubePresets);
};

#endif // TUBEPRESETMODEL_H
