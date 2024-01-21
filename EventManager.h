// EventManager class definition
#include "EventModel.h"
#include <queue>
#include <vector>
#include <chrono>

class EventManager {
private:
    std::vector<EventModel*> eventList;
    std::priority_queue<EventModel*> eventQueue;

public:
    // Method to add events
    void addEvent(EventModel *event);
    // Method to trigger events
    void triggerEvents(float time);
};
