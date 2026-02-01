#include "presetmodel.h"

#include <string>
#include <vector>

PresetModel::PresetModel()
{

}

std::string PresetModel::getName() const
{
    return this->name;
}

QColor PresetModel::getColor() const
{
    return color;
}

void PresetModel::setColor(const QColor &newColor)
{
    color = newColor;
}

void PresetModel::setName(const std::string newName)
{
    this->name = newName;
}


