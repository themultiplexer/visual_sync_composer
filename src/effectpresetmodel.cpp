#include "effectpresetmodel.h"
#include "qjsonobject.h"

EffectPresetModel::EffectPresetModel(std::string name, int id) {
    this->name = name;
    this->id = id;
    config.led_mode = 1;
    config.brightness = 10;
    config.parameter1 = 7;
    config.parameter2 = 6;
    config.parameter3 = 6;
    config.speed_factor = 4;
    config.offset = 0;
}

EffectPresetModel::EffectPresetModel(std::string name, int id, CONFIG_DATA data) {
    this->name = name;
    this->id = id;
    config = data;
}

QJsonObject EffectPresetModel::toJson() const {
    QJsonObject obj;
    obj["name"] = name.c_str();
    obj["id"] = id;
    obj["led_mode"] = config.led_mode;
    obj["speed_factor"] = config.speed_factor;
    obj["brightness"] = config.brightness;
    obj["parameter1"] = config.parameter1;
    obj["parameter2"] = config.parameter2;
    obj["parameter3"] = config.parameter3;
    obj["modifiers"] = config.modifiers;
    return obj;
}

EffectPresetModel* EffectPresetModel::fromJson(const QJsonObject &obj) {
    CONFIG_DATA preset;
    std::string name = obj["name"].toString().toStdString();
    preset.led_mode = obj["led_mode"].toInt();
    preset.speed_factor = obj["speed_factor"].toInt();
    preset.brightness = obj["brightness"].toInt();
    preset.parameter1 = obj["parameter1"].toInt();
    preset.parameter2 = obj["parameter2"].toInt();
    preset.parameter3 = obj["parameter3"].toInt();
    preset.modifiers = obj["modifiers"].toInt();
    preset.offset = 0;
    for (int i = 0; i < 32; i++){
        if (i % 2 == 0) {
            preset.pattern[i] = 0xFF;
        } else {
            preset.pattern[i] = 0x00;
        }
    }
    auto f = new EffectPresetModel(name, obj["id"].toInt(), preset);
    return f;
}
