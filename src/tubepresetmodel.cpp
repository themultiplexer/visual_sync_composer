#include "tubepresetmodel.h"
#include "qjsonobject.h"

TubePresetModel::TubePresetModel(std::string name, int id) {
    this->name = name;
    this->id = id;
    tubePresets = std::map<std::string, TubePreset>();
    this->pattern = PATTERN_DATA { {0} };

}

TubePresetModel::TubePresetModel(std::string name, int id, std::map<std::string, TubePreset> presets) {
    this->name = name;
    this->id = id;
    tubePresets = presets;
    this->pattern = pattern;
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
    QJsonObject json;
    json["name"] = QString::fromStdString(name);
    json["id"] = id;
    QJsonObject array;
    for (auto const& [id, preset] : tubePresets) {
        QJsonObject obj;
        obj["delay"] = preset.delay;
        obj["group"] = preset.group;
        //obj["pattern"] = QJsonArray(std::vector<>(preset.pattern));
        array[id.c_str()] = obj;
    }
    json["tubes"] = array;
    return json;
}

TubePresetModel* TubePresetModel::fromJson(const QJsonObject &obj) {
    std::map<std::string, TubePreset> presets = std::map<std::string, TubePreset>();
    QJsonObject tubes = obj["tubes"].toObject();
    foreach(const QString& key, tubes.keys()) {
        QJsonValue value = tubes.value(key);
        TubePreset p = TubePreset();
        p.delay = value["delay"].toInt();
        p.group = value["group"].toInt();

        PATTERN_DATA pattern;
        for (int i = 0; i < 32; i++){
            if (i % 2 == 0) {
                p.pattern.pattern[i] = 0xFF;
            } else {
                p.pattern.pattern[i] = 0x00;
            }
        }
        presets[key.toStdString()] = p;
    }
    auto f = new TubePresetModel(obj["name"].toString().toStdString(), obj["id"].toInt(), presets);
    return f;
}

