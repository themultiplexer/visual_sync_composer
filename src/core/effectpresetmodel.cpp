#include "effectpresetmodel.h"
#include "core/wifieventprocessor.h"
#include "qjsonobject.h"
#include <cstdint>
#include <string>

EffectPresetModel::EffectPresetModel(std::string name, int id) : preset("empty", 0) {
    this->name = name;
    this->id = id;
    for (CONFIG_DATA *data : {&config, &secondary_config}) {
        data->led_mode = 1;
        data->brightness = 10;
        data->parameter1 = 7;
        data->parameter2 = 6;
        data->parameter3 = 6;
        data->speed_factor = 4;
    }
    dmx_config = {0};
}

EffectPresetModel::EffectPresetModel(std::string name, int id, CONFIG_DATA data, CONFIG_DATA data2, DMX_DATA dmx_data, QColor color, TubePresetModel tubepreset) : preset(tubepreset) {
    this->name = name;
    this->id = id;
    this->config = data;
    this->secondary_config = data2;
    this->dmx_config = dmx_data;
    this->color = color;
}

TubePresetModel* EffectPresetModel::getPresets()
{
    return &preset;
}

void EffectPresetModel::setPresets(TubePresetModel newPresets)
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
    QJsonArray array;
    array << color.red() << color.green() << color.blue();
    obj.insert("button_color", array);

    int i = 0;
    std::tuple<const CONFIG_DATA *, QString> items[] = {
    {&config, "config1"}, {&secondary_config, "config2"}
    };
    for(const auto& [data, str] : items) {
        QJsonObject cfg;
        cfg["led_mode"] = data->led_mode;
        cfg["speed_factor"] = data->speed_factor;
        cfg["brightness"] = data->brightness;
        cfg["parameter1"] = data->parameter1;
        cfg["parameter2"] = data->parameter2;
        cfg["parameter3"] = data->parameter3;
        cfg["modifiers"] = data->modifiers;
        obj.insert(str, cfg);
        i++;
    }

    QJsonArray dmxarray;
    for (int i = 0; i < 10; i++) {
        dmxarray << dmx_config.channels[i];
    }
    obj.insert("dmx_config", dmxarray);


    return obj;
}

EffectPresetModel* EffectPresetModel::fromJson(const QJsonObject &obj) {
    CONFIG_DATA config1, config2;
    DMX_DATA dmx_data;
    std::string name = obj["name"].toString().toStdString();
    std::tuple<CONFIG_DATA *, QJsonValue> items[] = {
        {&config1, obj["config1"]}, {&config2, obj["config2"]}
    };
    for(const auto& [config, jsonobj] : items) {
        config->led_mode = jsonobj["led_mode"].toInt();
        config->speed_factor = jsonobj["speed_factor"].toInt();
        config->brightness = jsonobj["brightness"].toInt();
        config->parameter1 = jsonobj["parameter1"].toInt();
        config->parameter2 = jsonobj["parameter2"].toInt();
        config->parameter3 = jsonobj["parameter3"].toInt();
        config->modifiers = jsonobj["modifiers"].toInt();
    }

    QJsonArray dmxarray = obj["dmx_config"].toArray();
    for (int i = 0; i < 10; i++) {
        dmx_data.channels[i] = (uint8_t)(dmxarray[i].toInt());
    }

    QJsonArray array = obj["button_color"].toArray();
    QColor color = QColor(array[0].toInt(), array[1].toInt(), array[2].toInt());

    TubePresetModel *tubepreset = TubePresetModel::fromJson(obj["tubepresets"].toObject());

    auto f = new EffectPresetModel(name, obj["id"].toInt(), config1, config2, dmx_data, color, *tubepreset);
    return f;
}
