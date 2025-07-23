#ifndef EFFECTPRESETMODEL_H
#define EFFECTPRESETMODEL_H
#include "wifieventprocessor.h"
#include "presetmodel.h"
#include "tubepresetmodel.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>

class EffectPresetModel : public PresetModel
{
public:
    EffectPresetModel(std::string name, int id);
    EffectPresetModel(std::string name, int id, CONFIG_DATA data, PATTERN_DATA pattern, TubePresetModel presets);

    int id;
    struct CONFIG_DATA config;
    struct PATTERN_DATA pattern;
    TubePresetModel preset;

    QJsonObject toJson() const;
    static EffectPresetModel * fromJson(const QJsonObject &obj);
    CONFIG_DATA getConfig() const;
    void setConfig(const CONFIG_DATA &newConfig);
    const TubePresetModel* getPresets() const;
    void setPresets(const TubePresetModel newPresets);
};

#endif // EFFECTPRESETMODEL_H
