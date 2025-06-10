
#include <array>
#include <span>
#include <stdexcept>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if_arp.h>
#include <arpa/inet.h>
#include <assert.h>

#include "espnowsender.h"
#include "espnowtypes.h"

void espnowsender::set_interface(char* interface) {
	this->interface = (char*) malloc(strlen(interface)*sizeof(char));	
	strcpy(this->interface, interface);
}

void espnowsender::start() {
	struct sockaddr_ll s_dest_addr;
    struct ifreq ifr;
	
    int fd, 			//file descriptor
		ioctl_errno,	//ioctl errno
		bind_errno,		//bind errno
		filter_errno,	//attach filter errno
		priority_errno;	//Set priority errno

	bzero(&s_dest_addr, sizeof(s_dest_addr));
    bzero(&ifr, sizeof(ifr));
	
    fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    assert(fd != -1);

    strncpy((char *)ifr.ifr_name, this->interface, IFNAMSIZ); //interface

    ioctl_errno = ioctl(fd, SIOCGIFINDEX, &ifr);
    if (ioctl_errno >= 0) {
        throw std::invalid_argument( "received negative value" );
    }

    s_dest_addr.sll_family = PF_PACKET;
    s_dest_addr.sll_protocol = htons(ETH_P_ALL);
    s_dest_addr.sll_ifindex = ifr.ifr_ifindex;
    
    bind_errno = bind(fd, (struct sockaddr *)&s_dest_addr, sizeof(s_dest_addr));
    if (bind_errno >= 0) {
        throw std::invalid_argument( "received bind_errno value" );
    }

	priority_errno = setsockopt(fd, SOL_SOCKET, SO_PRIORITY, &(this->socket_priority), sizeof(this->socket_priority));
    if (priority_errno >= 0) {
        throw std::invalid_argument( "received priority_errno value" );
    }
	
	this->sock_fd = fd;
}

void espnowsender::stop() {
    if (this->sock_fd > 0) {
        close(this->sock_fd);
    }
}

void espnowsender::end() {
	stop();

	if(this->interface != NULL) {
		free(this->interface);
		this->interface = NULL;
	}
}

int espnowsender::send(uint8_t *payload, int len, std::array<uint8_t,6> dst_mac) {
    mypacket.set_dst_mac(dst_mac.data());
	uint8_t raw_bytes[LEN_RAWBYTES_MAX];

	//Not the most fastest way to do this : 
	//	copy the payload in the packet array and then copy it back into the buffer...
	this->mypacket.wlan.actionframe.content.set_length(len); 
	memcpy(this->mypacket.wlan.actionframe.content.payload, payload, len);

	int raw_len = mypacket.toBytes(raw_bytes, LEN_RAWBYTES_MAX);
	return sendto(this->sock_fd, raw_bytes, raw_len, 0, NULL, 0);
}

int espnowsender::send() {
	uint8_t raw_bytes[LEN_RAWBYTES_MAX];
	int raw_len = mypacket.toBytes(raw_bytes, LEN_RAWBYTES_MAX);
	return sendto(this->sock_fd, raw_bytes, raw_len, 0, NULL, 0);
}

