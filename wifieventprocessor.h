#ifndef WIFIEVENTPROCESSOR_H
#define WIFIEVENTPROCESSOR_H

#include "ESPNOW_manager.h"
#include "Timeline/EventProcessor.h"
#include <QDebug>
#include <QLabel>
#include <random>

struct PEAK_DATA {
    uint8_t hue;
};

struct CONFIG_DATA {
    uint8_t led_mode;
    uint8_t speed_factor;
    uint8_t brightness;
    uint8_t parameter1;
    uint8_t parameter2;
    uint8_t hue1;
    uint8_t hue2;
    uint8_t sat;
};

struct TEXT_DATA {
    char data[50];
};


class WifiTextEventReceiver
{
public:
    virtual void textEvent(const char *text) = 0;
};


class WifiEventProcessor : public EventProcessor
{
public:
    WifiEventProcessor();
    void textEvent(std::string data);
    void peakEvent();
    void sendConfig();

    CONFIG_DATA masterconfig;

    void registerReceiver(WifiTextEventReceiver *receiver);
private:
    std::vector<ESPNOW_manager *> handlers;
    std::vector<WifiTextEventReceiver *> receivers;
    std::random_device dev;
    std::mt19937 *rng;
    std::uniform_int_distribution<std::mt19937::result_type> *hueRandom;
};

#endif // WIFIEVENTPROCESSOR_H
