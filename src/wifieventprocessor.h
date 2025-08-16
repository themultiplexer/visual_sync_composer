#ifndef WIFIEVENTPROCESSOR_H
#define WIFIEVENTPROCESSOR_H

#include "espnowsender.h"
#include "timeline/EventProcessor.h"
#include <QDebug>
#include <QLabel>
#include <random>

struct PEAK_DATA {
    uint8_t hue;
    uint8_t sat;
    uint8_t group;
};

struct SYNC_REQUEST {
    uint8_t a;
    uint8_t b;
};

struct UPDATE_REQUEST {
    uint32_t a;
};

struct DMX_DATA {
    uint8_t channels[5];
};

struct CONFIG_DATA {
    uint8_t tube_id;
    uint8_t led_mode;
    uint8_t speed_factor;
    uint8_t brightness;
    uint8_t parameter1;
    uint8_t parameter2;
    uint8_t parameter3;
    uint8_t modifiers;
    uint8_t offset;
    uint8_t group;

    std::string toString()
    {
        return "[tube_id = " + std::to_string(tube_id) + "led_mode = " + std::to_string(led_mode) + ", offset = " + std::to_string(offset) + "]";
    }
};

struct SYNC_DATA {
    uint8_t group[32];
    uint8_t offset[32];
};

struct PATTERN_DATA {
    uint8_t pattern[32];

    std::string toString()
    {
        return "[led_mode = " + std::to_string(pattern[0]) + "]";
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
    WifiEventProcessor(std::array<uint8_t, 6> my_mac, std::string dev);
    void textEvent(std::string data);
    void peakEvent(uint8_t hue, uint8_t sat, uint8_t group);
    void sendConfig();
    void sendUpdateMessage();
    void registerReceiver(WifiTextEventReceiver *receiver);
    void initHandlers();
    void setTubeOffsets(const std::vector<int> &newTubeOffsets);
    void sendConfigTo(std::array<uint8_t, 6> dst_mac);
    void sendSync();
    void sendUpdateMessageTo(std::array<uint8_t, 6> dst_mac);
    CONFIG_DATA getMasterconfig() const;
    void setMasterconfig(const CONFIG_DATA &newMasterconfig);

    std::vector<int> getTubeGroups() const;
    void setTubeGroups(const std::vector<int> &newTubeGroups);
    void sendSyncConfig();
    void sendDmx(uint8_t hue, uint8_t sat, uint8_t brightness, uint8_t group);

private:
    void callback(std::array<uint8_t, 6> src_mac, std::span<uint8_t> data);

    espnowsender* handler;
    std::vector<WifiTextEventReceiver *> receivers;
    std::vector<int> getTubeOffsets() const;
    CONFIG_DATA masterconfig;
    std::array<uint8_t, 6> my_mac;
    std::string dev;
    std::vector<int> tubeOffsets;
    std::vector<int> tubeGroups;
};

#endif // WIFIEVENTPROCESSOR_H
