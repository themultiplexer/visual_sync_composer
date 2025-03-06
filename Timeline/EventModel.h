#ifndef EVENTMODEL_H
#define EVENTMODEL_H

#include <boost/serialization/access.hpp>
#include <ostream>
#include <iostream>

class Track;

class EventModel {

    friend class boost::serialization::access;
    template <class Archive> void serialize(Archive& ar, unsigned) {
        ar &start &duration &text &lane;
    }

    inline friend std::ostream& operator<<(std::ostream &out, EventModel & model) {
        out << model.start << " - " << model.duration;

        return out;
    }

public:
    EventModel() {}
    EventModel(Track *track);

    std::string name;
    float start, duration;
    int lane;
    std::string text;
};


#endif // EVENTMODEL_H
