#include "tubepresetmodel.h"
#include "qjsonobject.h"

TubePresetModel::TubePresetModel(std::string name, int index, int id) {
    this->name = name;
    this->index = index;
    this->id = id;
    tubePresets = std::map<std::string, TubePreset>();
    this->pattern = PATTERN_DATA { {0} };

}

TubePresetModel::TubePresetModel(std::string name, int index, int id, std::map<std::string, TubePreset> presets, QColor color) {
    this->name = name;
    this->index = index;
    this->id = id;
    tubePresets = presets;
    this->pattern = PATTERN_DATA { {0} };
    this->color = color;
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
    json["index"] = index;
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
    QJsonArray c;
    c << color.red() << color.green() << color.blue();
    json.insert("button_color", c);
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
    QJsonArray array = obj["button_color"].toArray();
    QColor color = QColor(array[0].toInt(), array[1].toInt(), array[2].toInt());
    auto f = new TubePresetModel(obj["name"].toString().toStdString(), obj["id"].toInt(), obj["id"].toInt(), presets, color);
    return f;
}

