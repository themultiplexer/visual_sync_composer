#ifndef TUBEPRESETMODEL_H
#define TUBEPRESETMODEL_H
#include "wifieventprocessor.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include "presetmodel.h"

class TubePresetModel : public PresetModel
{
public:
    TubePresetModel(std::string name, int id);
    TubePresetModel(std::string name, int id, CONFIG_DATA data);


    int id;
    struct CONFIG_DATA config;

    QJsonObject toJson() const;
    static TubePresetModel * fromJson(const QJsonObject &obj);

};

#endif // TUBEPRESETMODEL_H
