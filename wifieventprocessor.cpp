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

using namespace std;

//static uint8_t my_mac[6] = {0xF0, 0x79, 0x60, 0x18, 0x34, 0x3C};
static uint8_t my_mac[6] = {0xDC, 0x4E, 0xF4, 0x0A, 0x3F, 0x9F};
static uint8_t dest_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static uint8_t ESP_mac[6] = {0x3c, 0x61, 0x05, 0x14, 0x16, 0x5c};

uint8_t macs[3][8] = {{0x8C, 0xCE, 0x4E, 0xE3, 0x6B, 0xED}, {0x24, 0xA1, 0x60, 0x3D, 0x86, 0x4A}, {0x60, 0x01, 0x94, 0x96, 0xC4, 0x4A}};

void callback(uint8_t src_mac[6], uint8_t *data, int len) {
    if(len == 6) {
        CONFIG_DATA *d = reinterpret_cast<CONFIG_DATA*>(data);
        std::cout << "Got config: " << std::to_string(d->led_mode) << " ";
    }

    for (int i = 0; i < len; i++) {
        printf("%02hhX ", data[i]);
    }
    printf("\n");
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

    for (int i = 0; i < 3; ++i) {
        ESPNOW_manager *handler = new ESPNOW_manager((char*)"wlxdc4ef40a3f9f", DATARATE_1Mbps, CHANNEL_freq_1, my_mac, macs[i], false);
        handler->set_filter(ESP_mac, macs[1]);
        handler->set_recv_callback(&callback);
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
    for (auto handler : handlers) {
        handler->send(reinterpret_cast<uint8_t*>(&masterconfig), sizeof(masterconfig));
    }
}

void getRightPaddingString(std::string &str, int n, char padChar = ' ')
{
    if (n <= str.size()) {
        return;
    }

    str.insert(str.size(), n - str.size(), padChar);
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
