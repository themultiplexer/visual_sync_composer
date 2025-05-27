#ifndef EFFECTPRESETMODEL_H
#define EFFECTPRESETMODEL_H
#include "wifieventprocessor.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>

class EffectPresetModel
{
public:
    EffectPresetModel(QString name);
    EffectPresetModel(QString name, CONFIG_DATA data);

    QString name;
    struct CONFIG_DATA config;

    static void saveToJsonFile(const std::vector<EffectPresetModel*> &presets, const QString &filePath);
    QJsonObject toJson() const;
    static EffectPresetModel * fromJson(const QJsonObject &obj);
    static std::vector<EffectPresetModel*> readJson(const QString &filePath);
    QString getName() const;
    void setName(const QString newName);
};

#endif // EFFECTPRESETMODEL_H
