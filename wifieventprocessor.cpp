#include "wifieventprocessor.h"
#include <cstdint>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <sys/time.h>
#include <iostream>
#include <random>

#include "ESPNOW_manager.h"
#include "ESPNOW_types.h"
#include "espreceiver.h"

using namespace std;

//static uint8_t my_mac[6] = {0xF0, 0x79, 0x60, 0x18, 0x34, 0x3C};
static uint8_t my_mac[6] = {0xDC, 0x4E, 0xF4, 0x0A, 0x3F, 0x9F};
static uint8_t dest_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static uint8_t ESP_mac[6] = {0x3c, 0x61, 0x05, 0x14, 0x16, 0x5c};


uint8_t macs[11][8] = {{0x8C, 0xCE, 0x4E, 0xE3, 0x6B, 0xED},
                      {0x24, 0xA1, 0x60, 0x3D, 0x86, 0x4A},
                      {0x60, 0x01, 0x94, 0x96, 0xC4, 0x4A},
                      {0x24, 0xA1, 0x60, 0x3D, 0x87, 0x2F},
                      {0x24, 0xA1, 0x60, 0x3A, 0xCF, 0x56},
                      {0x24, 0xA1, 0x60, 0x3D, 0x48, 0x56},
                      {0x24, 0xA1, 0x60, 0x3D, 0x8B, 0x0A},
                      {0x24, 0xA1, 0x60, 0x3A, 0xA2, 0xDC},
                      {0x24, 0xA1, 0x60, 0x3A, 0x2B, 0xE9},
                      {0x8C, 0xCE, 0x4E, 0xE1, 0xDF, 0x2E},
                      {0x8C, 0xCE, 0x4E, 0xDE, 0xB3, 0xC7}}; //  8C:CE:4E:DE:B3:C7 8C:CE:4E:E1:DF:2E

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
        sendConfig();
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

    rng = new std::mt19937(dev());
    hueRandom = new std::uniform_int_distribution<std::mt19937::result_type>(0, 255);

}

void WifiEventProcessor::initHandlers() {

    espreceiver *receiver = new espreceiver("wlxdc4ef40a3f9f");
    receiver->set_recv_callback(std::bind(&WifiEventProcessor::callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    receiver->set_filter(my_mac);
    receiver->start();

    for (int i = 0; i < 1; ++i) {
        ESPNOW_manager *handler = new ESPNOW_manager((char*)"wlxdc4ef40a3f9f", DATARATE_1Mbps, CHANNEL_freq_1, my_mac, new uint8_t[6] {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, true);
        handler->start();
        handlers.push_back(handler);
    }
}

void WifiEventProcessor::peakEvent() {
    PEAK_DATA peak;
    peak.hue = (*hueRandom)(*rng);
    for (auto handler : handlers) {
        handler->send(reinterpret_cast<uint8_t*>(&peak), 1);
    }
}

void WifiEventProcessor::sendConfig()
{
    for (int i = 0; i < handlers.size(); i++) {
        auto handler = handlers[i];
        CONFIG_DATA tube_config = masterconfig;
        if (i < tubeOffsets.size()) {
            tube_config.offset = tubeOffsets[i];
        }
        handler->send(reinterpret_cast<uint8_t*>(&tube_config), sizeof(tube_config));
    }
}

void WifiEventProcessor::updateFirmware()
{
    UPDATE_DATA update;
    for (auto handler : handlers) {
        handler->send(reinterpret_cast<uint8_t*>(&update), sizeof(update));
    }
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

    for (auto handler : handlers) {
        handler->send(reinterpret_cast<uint8_t*>(&text), sizeof(text));
    }
}
