#ifndef WIFIEVENTPROCESSOR_H
#define WIFIEVENTPROCESSOR_H

#include "espnowsender.h"
#include "Timeline/EventProcessor.h"
#include <QDebug>
#include <QLabel>
#include <random>

struct PEAK_DATA {
    uint8_t hue;
};

struct UPDATE_DATA {
    uint8_t a;
    uint8_t b;
};

struct SYNC_DATA {
    uint8_t a;
    uint8_t b;
    uint8_t c;
};

struct CONFIG_DATA {
    uint8_t led_mode;
    uint8_t speed_factor;
    uint8_t brightness;
    uint8_t parameter1;
    uint8_t parameter2;
    uint8_t parameter3;
    uint8_t hue;
    uint8_t sat;
    uint8_t modifiers;
    uint8_t offset;
    uint8_t pattern[32];

    std::string toString()
    {
        return "[led_mode = " + std::to_string(led_mode) + ", offset = " + std::to_string(offset) + "]";
    }
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
    void peakEvent(int hue);
    void sendConfig();
    void updateFirmware();

    CONFIG_DATA masterconfig;

    void registerReceiver(WifiTextEventReceiver *receiver);
    void initHandlers();


    std::vector<int> getTubeOffsets() const;
    void setTubeOffsets(const std::vector<int> &newTubeOffsets);

    void sendConfigTo(uint8_t dst_mac[]);
    void sendSync();
private:
    void callback(uint8_t src_mac[6], uint8_t *data, int len);

    espnowsender* handler;
    std::vector<WifiTextEventReceiver *> receivers;
    void static_callback(uint8_t src_mac[], uint8_t *data, int len, void *userData);

    std::vector<int> tubeOffsets;
};

#endif // WIFIEVENTPROCESSOR_H
