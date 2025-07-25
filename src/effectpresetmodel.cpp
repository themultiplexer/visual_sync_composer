#include "effectpresetmodel.h"
#include "qjsonobject.h"

EffectPresetModel::EffectPresetModel(std::string name, int id) : preset("empty") {
    this->name = name;
    this->id = id;
    config.led_mode = 1;
    config.brightness = 10;
    config.parameter1 = 7;
    config.parameter2 = 6;
    config.parameter3 = 6;
    config.speed_factor = 4;
    config.offset = 0;
    this->pattern = PATTERN_DATA { {0} };
}

EffectPresetModel::EffectPresetModel(std::string name, int id, CONFIG_DATA config, PATTERN_DATA pattern, TubePresetModel tubepreset) : preset(tubepreset) {
    this->name = name;
    this->id = id;
    this->config = config;
    this->pattern = pattern;
}

const TubePresetModel* EffectPresetModel::getPresets() const
{
    return &preset;
}

void EffectPresetModel::setPresets(const TubePresetModel newPresets)
{
    preset = newPresets;
}

CONFIG_DATA EffectPresetModel::getConfig() const
{
    return config;
}

void EffectPresetModel::setConfig(const CONFIG_DATA &newConfig)
{
    config = newConfig;
}

QJsonObject EffectPresetModel::toJson() const {
    QJsonObject obj;
    QJsonObject tubes;
    obj["tubepresets"] = preset.toJson();
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
    CONFIG_DATA config;
    std::string name = obj["name"].toString().toStdString();
    config.led_mode = obj["led_mode"].toInt();
    config.speed_factor = obj["speed_factor"].toInt();
    config.brightness = obj["brightness"].toInt();
    config.parameter1 = obj["parameter1"].toInt();
    config.parameter2 = obj["parameter2"].toInt();
    config.parameter3 = obj["parameter3"].toInt();
    config.modifiers = obj["modifiers"].toInt();
    config.offset = 0;

    TubePresetModel *tubepreset = TubePresetModel::fromJson(obj["tubepresets"].toObject());

    PATTERN_DATA pattern;
    for (int i = 0; i < 32; i++){
        if (i % 2 == 0) {
            pattern.pattern[i] = 0xFF;
        } else {
            pattern.pattern[i] = 0x00;
        }
    }
    auto f = new EffectPresetModel(name, obj["id"].toInt(), config, pattern, *tubepreset);
    return f;
}
