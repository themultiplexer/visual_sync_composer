#include "EventModel.h"
#include "Track.h"

EventModel::EventModel(Track *track) {
    start = track->startTime;
    duration = track->duration;
    lane = (int)track->scenePos().y() / 35;
    text = track->text.toStdString();
}
