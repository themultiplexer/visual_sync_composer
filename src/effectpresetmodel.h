#ifndef EFFECTPRESETMODEL_H
#define EFFECTPRESETMODEL_H
#include "wifieventprocessor.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>

class EffectPresetModel
{
public:
    EffectPresetModel(std::string name, int id);
    EffectPresetModel(std::string name, int id, CONFIG_DATA data);

    std::string name;
    int id;
    struct CONFIG_DATA config;

    static void saveToJsonFile(const std::vector<EffectPresetModel*> &presets, const std::string &filePath);
    QJsonObject toJson() const;
    static EffectPresetModel * fromJson(const QJsonObject &obj);
    static std::vector<EffectPresetModel*> readJson(const std::string &filePath);
    std::string getName() const;
    void setName(const std::string newName);
};

#endif // EFFECTPRESETMODEL_H
