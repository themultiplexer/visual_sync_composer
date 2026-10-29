#include "effectpresetmodel.h"
#include "core/dmxfixture.h"
#include "core/wifieventprocessor.h"
#include "qjsonobject.h"
#include <cstdint>
#include <string>
#include <vector>

EffectPresetModel::EffectPresetModel(std::string name, int id) : preset("empty", 0), colors() {
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
    group_mode = 0;
    color_mode = 0;
}

EffectPresetModel::EffectPresetModel(std::string name, int id, CONFIG_DATA data, CONFIG_DATA data2, std::vector<DmxFixture> dmx_data, QColor color, int group_mode, int color_mode, std::vector<std::array<float, 2>> colors, TubePresetModel tubepreset) : preset(tubepreset) {
    this->name = name;
    this->id = id;
    this->config = data;
    this->secondary_config = data2;
    this->dmx_config = dmx_data;
    this->color = color;
}

TubePresetModel* EffectPresetModel::getCoordinations()
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

    QJsonArray jcolors;
    for (std::array<float, 2> color : colors) {
        QJsonArray array;
        array << color[0] << color[1];
        jcolors.push_back(array);
    }
    obj.insert("colors", jcolors);

    obj["group_mode"] = group_mode;
    obj["color_mode"] = color_mode;

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
    for (auto fixture : dmx_config) {
        QJsonObject fix;
        fix["name"] = fixture.name;
        fix["path"] = fixture.path;
        fix["mode"] = fixture.mode;
        fix["address"] = fixture.address;
        fix["group"] = fixture.group;
        qDebug() << "XX"  << fixture.group;
        fix["delay"] = fixture.delay;
        fix["program"] = fixture.program;
        QJsonArray sliderarray;
        for (auto slider : fixture.sliders) {
            sliderarray.push_back(slider.value());
        }
        fix["values"] = sliderarray;
        dmxarray.push_back(fix);
    }

    obj.insert("fixtures", dmxarray);


    return obj;
}

EffectPresetModel* EffectPresetModel::fromJson(const QJsonObject &obj) {
    CONFIG_DATA config1, config2;
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

    std::vector<DmxFixture> fixtures;
    QJsonArray dmxarray = obj["fixtures"].toArray();
    for (const auto fix : dmxarray) {
        auto obj = fix.toObject();
        DmxFixture fixture = loadFixture(obj["path"].toString(), obj["mode"].toString());
        const auto sliders = obj["values"].toArray();
        for (int i = 0; i < sliders.count(); i++) {
            if (i < fixture.sliders.count()) {
                fixture.sliders[i].setValue(sliders[i].toInt());
            }
        }
        fixture.address = obj["address"].toInt();
        fixture.group = obj["group"].toInt();
        fixture.delay = obj["delay"].toInt();
        fixture.program = obj["program"].toInt();
        fixtures.push_back(fixture);
    }

    QJsonArray array = obj["button_color"].toArray();
    QColor color = QColor(array[0].toInt(), array[1].toInt(), array[2].toInt());

    TubePresetModel *tubepreset = TubePresetModel::fromJson(obj["tubepresets"].toObject());

    QJsonArray jcolors = obj["colors"].toArray();

    std::vector<std::array<float, 2>> colors;

    for (QJsonValueRef obj : jcolors) {
        colors.push_back({static_cast<float>(obj[0].toDouble()), static_cast<float>(obj[1].toDouble()) });
    }

    int group_mode = obj["group_mode"].toInt();
    int color_mode = obj["color_mode"].toInt();

    auto f = new EffectPresetModel(name, obj["id"].toInt(), config1, config2, fixtures, color, group_mode, color_mode, colors, *tubepreset);
    return f;
}
