#include "wifieventprocessor.h"
#include <cstdint>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <sys/time.h>
#include <iostream>
#include <random>
#include "devicereqistry.h"
#include "espnowsender.h"
#include "espnowtypes.h"
#include "espreceiver.h"
#include "helper.h"

using namespace std;

//static uint8_t my_mac[6] = {0xF0, 0x79, 0x60, 0x18, 0x34, 0x3C};
static uint8_t my_mac[6] = {0xDC, 0x4E, 0xF4, 0x0A, 0x3F, 0x9F};



void WifiEventProcessor::callback(uint8_t src_mac[6], uint8_t *data, int len) {
    if(len == 6) {
        CONFIG_DATA *d = reinterpret_cast<CONFIG_DATA*>(data);
        std::cout << "Got config: " << std::to_string(d->led_mode) << " ";
    } if(len == 1) {
        std::cout << "Got hello from tube: ";
        for (int i = 0; i < 6; i++) {
            printf("%02hhX:", src_mac[i]);
        }
        std::cout << std::endl;
        //sendConfigTo(src_mac);
    } else {
        for (int i = 0; i < len; i++) {
            printf("%02hhX ", data[i]);
        }
        printf("\n");
    }
}

std::vector<int> WifiEventProcessor::getTubeOffsets() const
{
    return tubeOffsets;
}

void WifiEventProcessor::setTubeOffsets(const std::vector<int> &newTubeOffsets)
{
    qDebug() << tubeOffsets;
    tubeOffsets = newTubeOffsets;
}

WifiEventProcessor::WifiEventProcessor() {
    masterconfig = CONFIG_DATA {
        0,
        255,
        5,
        20,
        10,
        0,
        0
    };
}

void WifiEventProcessor::initHandlers() {

    espreceiver *receiver = new espreceiver("wlxdc4ef40a3f9f");
    receiver->set_recv_callback(std::bind(&WifiEventProcessor::callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    receiver->set_filter(my_mac);
    receiver->start();

    handler = new espnowsender((char*)"wlxdc4ef40a3f9f", DATARATE_1Mbps, CHANNEL_freq_1, my_mac);
    handler->start();
}

void WifiEventProcessor::peakEvent(uint8_t hue) {
    PEAK_DATA peak;
    peak.hue = hue;
    handler->send(reinterpret_cast<uint8_t*>(&peak), 1, new uint8_t[6] {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF});
}

void WifiEventProcessor::sendConfig()
{
    auto macs = devicereqistry::macs();
    for (int i = 0; i < devicereqistry::macs().size(); i++) {
        CONFIG_DATA tube_config = masterconfig;
        if (i < tubeOffsets.size()) {
            tube_config.offset = tubeOffsets[i];
        }
        handler->send(reinterpret_cast<uint8_t*>(&tube_config), sizeof(tube_config), macs[i].data());
        std::cout << "Sending" << tube_config.toString() << " to " << arrayToString(macs[i]) << std::endl;
    }
    sendSync();
}

void WifiEventProcessor::sendConfigTo(uint8_t dst_mac[6])
{
    std::array<uint8_t, 6> mac_array;
    std::copy(dst_mac, dst_mac + 6, mac_array.begin());
    CONFIG_DATA tube_config = masterconfig;
    auto macs = devicereqistry::macs();
    auto it = std::find(macs.begin(), macs.end(), mac_array);
    if (it != macs.end()) {
        int i = std::distance(macs.begin(), it);
        if (i < tubeOffsets.size()) {
            tube_config.offset = tubeOffsets[i];
        }
    }
    handler->send(reinterpret_cast<uint8_t*>(&tube_config), sizeof(CONFIG_DATA), mac_array.data());
    std::cout << "Sending" << tube_config.toString() << " to " << arrayToString(mac_array) << std::endl;
    sendSync();
}

void WifiEventProcessor::sendSync()
{
    SYNC_DATA sync;
    handler->send(reinterpret_cast<uint8_t*>(&sync), sizeof(SYNC_DATA), new uint8_t[6] {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF});
}

void WifiEventProcessor::sendUpdateMessage()
{
    UPDATE_DATA update;
    handler->send(reinterpret_cast<uint8_t*>(&update), sizeof(UPDATE_DATA), new uint8_t[6] {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF});
}

void WifiEventProcessor::sendUpdateMessageTo(const uint8_t dst_mac[6])
{
    UPDATE_DATA update;
    handler->send(reinterpret_cast<uint8_t*>(&update), sizeof(UPDATE_DATA), dst_mac);
}

void WifiEventProcessor::registerReceiver(WifiTextEventReceiver *receiver) {
    receivers.push_back(receiver);
}

void WifiEventProcessor::textEvent(std::string data) {
    qDebug() << data.c_str();

    for (WifiTextEventReceiver *receiver : receivers) {
        receiver->textEvent(data.c_str());
    }

    TEXT_DATA text;
    strcpy(text.data, data.c_str());

    handler->send(reinterpret_cast<uint8_t*>(&text), sizeof(text), new uint8_t[6] {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF});
}
