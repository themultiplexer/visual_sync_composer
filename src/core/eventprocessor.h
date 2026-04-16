#ifndef EVENTPROCESSOR_H
#define EVENTPROCESSOR_H

#include <string>

class EventProcessor
{
public:
    virtual ~EventProcessor() {}
    virtual void textEvent(std::string data) = 0;
};

#endif // EVENTPROCESSOR_H
