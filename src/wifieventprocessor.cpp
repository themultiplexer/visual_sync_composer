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
        sendHelloTo(src_mac);
        sendConfigTo(src_mac);
    } else {
        for (int i = 0; i < data.size(); i++) {
            printf("adsfsadf%02hhX ", data[i]);
        }
        printf("\n");
    }
}

std::vector<int> WifiEventProcessor::getTubeOffsets() const
{
    return tubeOffsets;
}

std::vector<int> WifiEventProcessor::getTubeGroups() const
{
    return tubeGroups;
}

void WifiEventProcessor::setTubeGroups(const std::vector<int> &newTubeGroups)
{
    tubeGroups = newTubeGroups;
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

static uint8_t brightness;

void WifiEventProcessor::sendPalette(std::array<uint8_t, 8> palette) {
    PALETTE_DATA pal;
    memcpy(pal.palette, palette.data(), 8);
    handler->send(reinterpret_cast<uint8_t*>(&pal), sizeof(PEAK_DATA), {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF});
}

void WifiEventProcessor::peakEvent(uint8_t hue, uint8_t sat, uint8_t group) {
    PEAK_DATA peak;
    peak.hue = hue;
    peak.sat = sat;
    peak.group = group;
    handler->send(reinterpret_cast<uint8_t*>(&peak), sizeof(PEAK_DATA), {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF});
}

void WifiEventProcessor::sendDmx(std::vector<double> channels) {
    DMX_DATA dmx;
    std::array<uint8_t, 5> bytes{};
    for (size_t i = 0; i < bytes.size(); ++i) {
        bytes[i] = channels[i] * 255.0f;
    }

    handler->send(reinterpret_cast<uint8_t*>(bytes.data()), bytes.size(), {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF});
}


void WifiEventProcessor::sendConfig()
{
    sendConfigTo({0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF});
}

void WifiEventProcessor::sendSyncConfig()
{
    qDebug() << "Sending Sync Config";
    auto macs = devicereqistry::macs();
    SYNC_DATA sync_config = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};
    for (int i = 0; i < tubeOffsets.size(); i++) {
        sync_config.offset[i] = tubeOffsets[i];
    }
    for (int i = 0; i < tubeGroups.size(); i++) {
        sync_config.group[i] = tubeGroups[i];
    }
    handler->send(reinterpret_cast<uint8_t*>(&sync_config), sizeof(SYNC_DATA), {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF});
}

void WifiEventProcessor::sendConfigTo(std::array<uint8_t, 6> dst_mac)
{
    CONFIG_DATA tube_config = masterconfig;
    handler->send(reinterpret_cast<uint8_t*>(&tube_config), sizeof(CONFIG_DATA), dst_mac);
    std::cout << "Sending" << tube_config.toString() << " to " << arrayToString(dst_mac) << std::endl;
}

void WifiEventProcessor::sendHelloToAll() {
    std::cout << "Sending hello to all";
    auto macs = devicereqistry::macs();
    for (int i = 0; i < macs.size(); ++i) {
        sendHelloIdTo(i, macs[i]);
    }
}

void WifiEventProcessor::sendHelloIdTo(uint8_t i, std::array<uint8_t, 6> dst_mac)
{
    HELLO_DATA hello = { i };
    handler->send(reinterpret_cast<uint8_t*>(&hello), sizeof(HELLO_DATA), dst_mac);
    std::cout << "Sending hello to " << +i << arrayToString(dst_mac) << std::endl;
}

void WifiEventProcessor::sendHelloTo(std::array<uint8_t, 6> dst_mac)
{
    auto macs = devicereqistry::macs();
    auto it = std::find(macs.begin(), macs.end(), dst_mac);
    if (it != macs.end()) {
        int i = std::distance(macs.begin(), it);
        if (i < tubeOffsets.size()) {
            sendHelloIdTo(i, dst_mac);
        }
    }

}

void WifiEventProcessor::sendUpdateMessage()
{
    UPDATE_REQUEST update;
    handler->send(reinterpret_cast<uint8_t*>(&update), sizeof(UPDATE_REQUEST), {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF});
}

void WifiEventProcessor::sendUpdateMessageTo(std::array<uint8_t, 6> dst_mac)
{
    UPDATE_REQUEST update;
    handler->send(reinterpret_cast<uint8_t*>(&update), sizeof(UPDATE_REQUEST), dst_mac);
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
