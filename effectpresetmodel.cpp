#include "effectpresetmodel.h"
#include "qjsonobject.h"

EffectPresetModel::EffectPresetModel(QString name) {
    this->name = name;
    config.led_mode = 1;
    config.brightness = 10;
    config.hue = 9;
    config.parameter1 = 7;
    config.parameter2 = 6;
    config.parameter3 = 6;
    config.sat = 5;
    config.speed_factor = 4;
    config.offset = 0;
}

EffectPresetModel::EffectPresetModel(QString name, CONFIG_DATA data) {
    this->name = name;
    config = data;
}

QString EffectPresetModel::getName() const
{
    return this->name;
}

void EffectPresetModel::setName(const QString newName)
{
    this->name = newName;
}

QJsonObject EffectPresetModel::toJson() const {
    QJsonObject obj;
    obj["name"] = name;
    obj["led_mode"] = config.led_mode;
    obj["speed_factor"] = config.speed_factor;
    obj["brightness"] = config.brightness;
    obj["parameter1"] = config.parameter1;
    obj["parameter2"] = config.parameter2;
    obj["hue"] = config.hue;
    obj["parameter3"] = config.parameter3;
    obj["modifiers"] = config.modifiers;
    obj["sat"] = config.sat;
    return obj;
}

EffectPresetModel* EffectPresetModel::fromJson(const QJsonObject &obj) {
    CONFIG_DATA preset;
    QString name = obj["name"].toString();
    preset.led_mode = obj["led_mode"].toInt();
    preset.speed_factor = obj["speed_factor"].toInt();
    preset.brightness = obj["brightness"].toInt();
    preset.parameter1 = obj["parameter1"].toInt();
    preset.parameter2 = obj["parameter2"].toInt();
    preset.hue = obj["hue"].toInt();
    preset.parameter3 = obj["parameter3"].toInt();
    preset.modifiers = obj["modifiers"].toInt();
    preset.sat = obj["sat"].toInt();
    preset.offset = 0;
    for (int i = 0; i < 32; i++){
        if (i % 2 == 0) {
            preset.pattern[i] = 0xFF;
        } else {
            preset.pattern[i] = 0x00;
        }
    }
    auto f = new EffectPresetModel(name, preset);
    return f;
}

std::vector<EffectPresetModel*> EffectPresetModel::readJson(const QString &filePath) {
    std::vector<EffectPresetModel*> presets;
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray jsonData = file.readAll();
        file.close();

        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
        if (jsonDoc.isArray()) {
            QJsonArray jsonArray = jsonDoc.array();
            for (const auto &value : jsonArray) {
                if (value.isObject()) {
                    presets.push_back(EffectPresetModel::fromJson(value.toObject()));
                }
            }
        }
    } else {
        qDebug() << "Failed to open file for reading";
        for (int i = 0; i < 100; ++i) {
            presets.push_back(new EffectPresetModel("<empty>"));
        }
    }
    return presets;
}

// Function to save an array of structs to a JSON file
void EffectPresetModel::saveToJsonFile(const std::vector<EffectPresetModel*> &presets, const QString &filePath) {
    QJsonArray jsonArray;
    for (const auto &preset : presets) {
        jsonArray.append(preset->toJson());
    }

    QJsonDocument jsonDoc(jsonArray);
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(jsonDoc.toJson());
        file.close();
        qDebug() << "JSON saved to" << filePath;
    } else {
        qDebug() << "Failed to open file for writing";
    }
}
