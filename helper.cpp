#include "helper.h"

std::vector<CONFIG_DATA> Helper::readJson() {
    std::vector<CONFIG_DATA> effects;
    // Path to your JSON file
    QString filePath = "presets.json";  // Update the path as needed

    // Open the JSON file
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open file for reading:" << filePath;
        return effects;
    }

    // Read the content of the file
    QByteArray fileData = file.readAll();
    file.close();

    // Parse the JSON data
    QJsonDocument doc = QJsonDocument::fromJson(fileData);

    if (doc.isNull()) {
        qWarning() << "Failed to parse JSON.";
        return effects;
    }

    // Check if it's a JSON object or array
    if (doc.isObject()) {
        QJsonObject jsonObject = doc.object();

        // Example: Access a specific field (assuming you have a field "name")
        if (jsonObject.contains("name") && jsonObject["name"].isString()) {
            QString name = jsonObject["name"].toString();
            qDebug() << "Name:" << name;
        }
    } else if (doc.isArray()) {
        QJsonArray jsonArray = doc.array();

        // Example: Loop through JSON array
        for (int i = 0; i < jsonArray.size(); ++i) {
            QJsonObject jsonObject = jsonArray[i].toObject();
            qDebug() << "Object" << i << ":" << jsonObject;
        }
    } else {
        qWarning() << "JSON is neither an object nor an array.";
    }
    return effects;
}
