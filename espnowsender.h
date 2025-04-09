#ifndef ESPNOW_manager_H
#define ESPNOW_manager_H

#include <stdint.h>
#include <linux/filter.h>
#include <string.h>
#include <sys/types.h>
#include <stdint.h>
#include <pthread.h>

#include "espnowtypes.h"

class espnowsender {
	public:
    espnowsender() {
			default_values();
		}

    espnowsender(char* interface) {
			default_values();
			set_interface(interface);
		}

    espnowsender(char* interface, uint8_t datarate, uint16_t channel_freq, uint8_t src_mac[6]) {
			default_values();
			set_interface(interface);
			set_channel(channel_freq);
			set_datarate(datarate);
			set_src_mac(src_mac);
		}

		void set_interface(char* interface);
		
		void start();
		void stop();
		void end();
		
		//int send(ESPNOW_packet p);
        int send(uint8_t *payload, int len, uint8_t dst_mac[6]);
		int send();
		
		void set_channel(uint16_t channel_freq) { mypacket.set_channel(channel_freq); }
		void set_datarate(uint8_t datarate) { mypacket.set_datarate(datarate); }
		void set_src_mac(uint8_t src_mac[6]) { mypacket.set_src_mac(src_mac); }	

		ESPNOW_packet mypacket;
	private:
		int sock_fd;
		int socket_priority;
		char* interface;

		void default_values() {
			socket_priority = 7; //Priority

		}

};



#endif
