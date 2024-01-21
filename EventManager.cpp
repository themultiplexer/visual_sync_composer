// EventManager class definition
#include "EventManager.h"

void EventManager::addEvent(EventModel *event) {
    eventList.push_back(event);
    eventQueue.push(event);
}

// Method to trigger events
void EventManager::triggerEvents(float time) {
    while (!eventQueue.empty() && eventQueue.top()->start <= time) {
        EventModel *currentEvent = eventQueue.top();
        eventQueue.pop();
        std::cout << "Triggering event: " << currentEvent->name << std::endl;
    }
}
