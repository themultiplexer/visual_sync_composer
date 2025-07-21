#ifndef EFFECTPRESETMODEL_H
#define EFFECTPRESETMODEL_H
#include "wifieventprocessor.h"
#include "presetmodel.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>

class EffectPresetModel : public PresetModel
{
public:
    EffectPresetModel(std::string name, int id);
    EffectPresetModel(std::string name, int id, CONFIG_DATA data, PATTERN_DATA pattern);

    int id;
    struct CONFIG_DATA config;
    struct PATTERN_DATA pattern;

    QJsonObject toJson() const;
    static EffectPresetModel * fromJson(const QJsonObject &obj);
};

#endif // EFFECTPRESETMODEL_H
