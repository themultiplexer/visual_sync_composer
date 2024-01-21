#ifndef PROJECTMODEL_H
#define PROJECTMODEL_H

#include "EventModel.h"
#include <boost/serialization/access.hpp>
#include <vector>
#include <iostream>


class ProjectModel {
    friend class boost::serialization::access;
    template <class Archive> void serialize(Archive& ar, unsigned) {
        ar &name &events;
    }

    inline friend std::ostream& operator<<(std::ostream &out, ProjectModel &project) {
        out << "Project: " << std::endl;
        out << project.name << ':' << std::endl;
        for (EventModel e : project.events) {
            out << e  << std::endl;
        }
        return out;
    }

public:
    std::vector<EventModel> events;
    std::string name;
};

#endif // PROJECTMODEL_H
