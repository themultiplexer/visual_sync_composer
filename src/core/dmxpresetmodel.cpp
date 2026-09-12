#include "dmxpresetmodel.h"
#include "qjsonobject.h"

DmxPresetModel::DmxPresetModel(std::string name, int index, int id) {
    this->name = name;
    this->index = index;
    this->id = id;
    tubePresets = std::map<std::string, DmxPreset>();
    this->pattern = PATTERN_DATA { {0} };

}

DmxPresetModel::DmxPresetModel(std::string name, int index, int id, std::map<std::string, DmxPreset> presets, QColor color) {
    this->name = name;
    this->index = index;
    this->id = id;
    tubePresets = presets;
    this->pattern = PATTERN_DATA { {0} };
    this->color = color;
}

std::map<std::string, DmxPreset> DmxPresetModel::getTubePresets() const
{
    return tubePresets;
}

void DmxPresetModel::setTubePresets(const std::map<std::string, DmxPreset> &newTubePresets)
{
    tubePresets = newTubePresets;
}

QJsonObject DmxPresetModel::toJson() const {
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

DmxPresetModel* DmxPresetModel::fromJson(const QJsonObject &obj) {
    std::map<std::string, DmxPreset> presets = std::map<std::string, DmxPreset>();
    QJsonObject tubes = obj["tubes"].toObject();
    foreach(const QString& key, tubes.keys()) {
        QJsonValue value = tubes.value(key);
        DmxPreset p = DmxPreset();
        p.delay = value["delay"].toInt();
        p.group = value["group"].toInt();
        presets[key.toStdString()] = p;
    }
    QJsonArray array = obj["button_color"].toArray();
    QColor color = QColor(array[0].toInt(), array[1].toInt(), array[2].toInt());
    auto f = new DmxPresetModel(obj["name"].toString().toStdString(), obj["id"].toInt(), obj["id"].toInt(), presets, color);
    return f;
}

