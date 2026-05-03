#ifndef FIXTUREPRESETMODEL_H
#define FIXTUREPRESETMODEL_H
#include "wifieventprocessor.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <map>
#include <qnamespace.h>
#include "presetmodel.h"

class FixturePreset {
public:
    int delay;
    int group;
    PATTERN_DATA pattern;
};

class FixturePresetModel : public PresetModel
{
public:
    FixturePresetModel(std::string name, int index, int id = 0, QColor color = Qt::transparent);

    int index, id;

    QJsonObject toJson() const;
    static FixturePresetModel * fromJson(const QJsonObject &obj);
};

#endif // FIXTUREPRESETMODEL_H
