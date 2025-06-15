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

void WifiEventProcessor::callback(std::array<uint8_t, 6> src_mac, std::span<uint8_t> data) {
    if(data.size() == 6) {
        CONFIG_DATA *d = reinterpret_cast<CONFIG_DATA*>(data.data());
        std::cout << "Got config: " << std::to_string(d->led_mode) << " ";
    } if(data.size() == 1) {
        std::cout << "Got hello from tube: " << arrayToString(src_mac) << std::endl;
        sendConfigTo(src_mac);
    } else {
        for (int i = 0; i < data.size(); i++) {
            printf("%02hhX ", data[i]);
        }
        printf("\n");
    }
}

std::vector<int> WifiEventProcessor::getTubeOffsets() const
{
    return tubeOffsets;
}

CONFIG_DATA WifiEventProcessor::getMasterconfig() const
{
    return masterconfig;
}

void WifiEventProcessor::setMasterconfig(const CONFIG_DATA &newMasterconfig)
{
    masterconfig = newMasterconfig;
}

void WifiEventProcessor::setTubeOffsets(const std::vector<int> &newTubeOffsets)
{
    qDebug() << tubeOffsets;
    tubeOffsets = newTubeOffsets;
}

WifiEventProcessor::WifiEventProcessor(std::array<uint8_t, 6> my_mac, std::string dev) {
    masterconfig = CONFIG_DATA {
        0,
        255,
        5,
        20,
        10,
        0,
        0
    };
    this->my_mac = my_mac;
    this->dev = dev;
}

void WifiEventProcessor::initHandlers() {
    try {
        espreceiver *receiver = new espreceiver(dev);
        receiver->set_recv_callback(std::bind(&WifiEventProcessor::callback, this, std::placeholders::_1, std::placeholders::_2));
        receiver->set_filter(&my_mac);
        receiver->start();
    } catch (...) {
        std::cout << "Error while initing receiver" << std::endl;
    }

    try {
        handler = new espnowsender((char*)dev.c_str(), DATARATE_1Mbps, CHANNEL_freq_1, my_mac);
        handler->start();
    } catch (...) {
        std::cout << "Error while initing sender" << std::endl;
    }
}

void WifiEventProcessor::peakEvent(uint8_t hue) {
    PEAK_DATA peak;
    peak.hue = hue;
    handler->send(reinterpret_cast<uint8_t*>(&peak), 1, {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF});
}

void WifiEventProcessor::sendConfig()
{
    auto macs = devicereqistry::macs();
    for (int i = 0; i < devicereqistry::macs().size(); i++) {
        CONFIG_DATA tube_config = masterconfig;
        if (i < tubeOffsets.size()) {
            tube_config.offset = tubeOffsets[i];
        }
        handler->send(reinterpret_cast<uint8_t*>(&tube_config), sizeof(tube_config), macs[i]);
        std::cout << "Sending" << tube_config.toString() << " to " << arrayToString(macs[i]) << std::endl;
    }
    sendSync();
}

void WifiEventProcessor::sendConfigTo(std::array<uint8_t, 6> dst_mac)
{
    CONFIG_DATA tube_config = masterconfig;
    auto macs = devicereqistry::macs();
    auto it = std::find(macs.begin(), macs.end(), dst_mac);
    if (it != macs.end()) {
        int i = std::distance(macs.begin(), it);
        if (i < tubeOffsets.size()) {
            tube_config.offset = tubeOffsets[i];
        }
    }
    handler->send(reinterpret_cast<uint8_t*>(&tube_config), sizeof(CONFIG_DATA), dst_mac);
    std::cout << "Sending" << tube_config.toString() << " to " << arrayToString(dst_mac) << std::endl;
    sendSync();
}

void WifiEventProcessor::sendSync()
{
    SYNC_DATA sync;
    handler->send(reinterpret_cast<uint8_t*>(&sync), sizeof(SYNC_DATA), {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF});
}

void WifiEventProcessor::sendUpdateMessage()
{
    UPDATE_DATA update;
    handler->send(reinterpret_cast<uint8_t*>(&update), sizeof(UPDATE_DATA), {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF});
}

void WifiEventProcessor::sendUpdateMessageTo(std::array<uint8_t, 6> dst_mac)
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

    handler->send(reinterpret_cast<uint8_t*>(&text), sizeof(text), {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF});
}
