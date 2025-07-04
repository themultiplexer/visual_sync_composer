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

void PresetModel::setName(const std::string newName)
{
    this->name = newName;
}


