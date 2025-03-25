#include "netdevice.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <net/if.h>
#include <linux/wireless.h>
#include <fcntl.h>

NetDevice::NetDevice(const char *interface) {
    this->interface = interface;
}

bool NetDevice::setInterface(bool up) {
    struct ifreq ifr;
    int sockfd;

    // Open a socket to communicate with the network device
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Zero out the ifreq struct
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, interface, IFNAMSIZ);

    // Get the current flags for the interface
    if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) == -1) {
        perror("IOCTL failed to get flags");
        close(sockfd);
        exit(EXIT_FAILURE);
    }


    if (up) {
        ifr.ifr_flags |= IFF_UP;
    } else {
        ifr.ifr_flags &= ~IFF_UP;
    }

    // Set the new flags using SIOCSIFFLAGS
    if (ioctl(sockfd, SIOCSIFFLAGS, &ifr) == -1) {
        perror("IOCTL failed to set flags");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Interface %s is now %s.\n", up ? "up" : "down", interface);

    // Close the socket
    close(sockfd);
}

bool NetDevice::enableMonitorMode() {
    struct iwreq request;
    int sockfd;

    // Open a socket to communicate with the network device
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        return false;
    }

    // Zero out the request struct
    memset(&request, 0, sizeof(request));
    strncpy(request.ifr_name, interface, IFNAMSIZ);

    // Set the mode to monitor mode
    request.u.mode = IW_MODE_MONITOR;

    // Perform the ioctl to change the mode of the interface
    if (ioctl(sockfd, SIOCSIWMODE, &request) == -1) {
        perror("IOCTL failed");
        close(sockfd);
        return false;
    }

    printf("Monitor mode set on %s\n", interface);

    // Close the socket
    close(sockfd);
    return true;
}
