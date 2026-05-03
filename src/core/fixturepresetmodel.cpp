#include "core/fixturepresetmodel.h"
#include "qjsonobject.h"


FixturePresetModel::FixturePresetModel(std::string name, int index, int id, QColor color) {
    this->name = name;
    this->index = index;
    this->id = id;
    this->color = color;
}

QJsonObject FixturePresetModel::toJson() const {
    QJsonObject json;
    json["name"] = QString::fromStdString(name);
    json["index"] = index;
    json["id"] = id;
    QJsonArray c;
    c << color.red() << color.green() << color.blue();
    json.insert("button_color", c);
    return json;
}

FixturePresetModel* FixturePresetModel::fromJson(const QJsonObject &obj) {
    std::map<std::string, FixturePreset> presets = std::map<std::string, FixturePreset>();
    QJsonArray array = obj["button_color"].toArray();
    QColor color = QColor(array[0].toInt(), array[1].toInt(), array[2].toInt());
    auto f = new FixturePresetModel(obj["name"].toString().toStdString(), obj["index"].toInt(), obj["id"].toInt(), color);
    return f;
}

