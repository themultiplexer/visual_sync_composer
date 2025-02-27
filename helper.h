#include "wifieventprocessor.h"
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

#ifndef HELPER_H
#define HELPER_H

class Helper
{
public:
    static std::vector<CONFIG_DATA> readJson();
};

#endif // HELPER_H
