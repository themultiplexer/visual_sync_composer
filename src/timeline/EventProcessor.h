#ifndef EVENTPROCESSOR_H
#define EVENTPROCESSOR_H

#include "EventModel.h"

class EventProcessor
{
public:
    virtual ~EventProcessor() {}
    virtual void textEvent(std::string data) = 0;
};

#endif // EVENTPROCESSOR_H
