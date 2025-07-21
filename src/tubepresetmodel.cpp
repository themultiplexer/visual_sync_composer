#include "tubepresetmodel.h"
#include "qjsonobject.h"

TubePresetModel::TubePresetModel(std::string name, int id) {
    this->name = name;
    this->id = id;
    tubePresets = std::map<std::string, TubePreset>();
}

TubePresetModel::TubePresetModel(std::string name, int id, std::map<std::string, TubePreset> presets) {
    this->name = name;
    this->id = id;
    tubePresets = presets;
}

std::map<std::string, TubePreset> TubePresetModel::getTubePresets() const
{
    return tubePresets;
}

void TubePresetModel::setTubePresets(const std::map<std::string, TubePreset> &newTubePresets)
{
    tubePresets = newTubePresets;
}

QJsonObject TubePresetModel::toJson() const {
    QJsonObject preset;
    preset["name"] = QString::fromStdString(name);
    preset["id"] = id;
    QJsonObject array;
    for (auto const& [id, preset] : tubePresets) {
        QJsonObject obj;
        obj["delay"] = preset.delay;
        obj["group"] = preset.group;
        array[id.c_str()] = obj;
    }
    preset["tubes"] = array;
    return preset;
}

TubePresetModel* TubePresetModel::fromJson(const QJsonObject &obj) {
    std::map<std::string, TubePreset> presets = std::map<std::string, TubePreset>();
    QJsonObject tubes = obj["tubes"].toObject();
    foreach(const QString& key, tubes.keys()) {
        QJsonValue value = tubes.value(key);
        TubePreset p = TubePreset();
        p.delay = value["delay"].toInt();
        p.group = value["group"].toInt();
        presets[key.toStdString()] = p;
    }
    auto f = new TubePresetModel(obj["name"].toString().toStdString(), obj["id"].toInt(), presets);
    return f;
}

