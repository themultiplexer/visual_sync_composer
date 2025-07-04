#ifndef PRESETMODEL_H
#define PRESETMODEL_H


#include "qjsonobject.h"
#include "qjsonarray.h"
#include "qjsondocument.h"
#include "qfile.h"
#include <string>
#include <vector>

class PresetModel
{
public:
    PresetModel();

    std::string name;
    std::string getName() const;
    void setName(const std::string newName);


    std::vector<PresetModel *> readJson(const std::string &filePath);
    void saveToJsonFile(const std::vector<PresetModel *> &presets, const std::string &filePath);

    template <typename ModelType>
    static std::vector<ModelType*> readJson(const std::string &filePath) {
        std::vector<ModelType*> presets;
        QFile file(filePath.c_str());
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray jsonData = file.readAll();
            file.close();

            QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
            if (jsonDoc.isArray()) {
                QJsonArray jsonArray = jsonDoc.array();
                for (const auto &value : jsonArray) {
                    if (value.isObject()) {
                        presets.push_back(ModelType::fromJson(value.toObject()));
                    }
                }
            }
        } else {
            qDebug() << "Failed to open file for reading";
            for (int i = 0; i < 100; ++i) {
                presets.push_back(new ModelType("empty", i));
            }
        }
        return presets;
    }

    // Function to save an array of structs to a JSON file
    template <typename ModelType>
    static void saveToJsonFile(const std::vector<ModelType*> &presets, const std::string &filePath) {
        QJsonArray jsonArray;
        for (const auto &preset : presets) {
            jsonArray.append(preset->toJson());
        }

        QJsonDocument jsonDoc(jsonArray);
        QFile file(filePath.c_str());
        if (file.open(QIODevice::WriteOnly)) {
            file.write(jsonDoc.toJson());
            file.close();
            qDebug() << "JSON saved to" << filePath.c_str();
        } else {
            qDebug() << "Failed to open file for writing";
        }
    }
};

#endif // PRESETMODEL_H
