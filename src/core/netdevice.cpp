#ifdef __APPLE__
#else
#include <net/if.h>
#include <linux/wireless.h>
#endif

#include "netdevice.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <fcntl.h>

NetDevice::NetDevice(const char *interface) {
    this->interface = interface;
}


#ifdef __APPLE__
bool NetDevice::setInterface(bool up) {
    return true;
}

bool NetDevice::enableMonitorMode() {
    return true;
}

bool NetDevice::checkInterface() {
    return true;
}

#else

bool NetDevice::setInterface(bool up) {
    struct ifreq ifr;
    int sockfd;

    // Open a socket to communicate with the network device
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        return false;
    }

    // Zero out the ifreq struct
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, interface, IFNAMSIZ);

    // Get the current flags for the interface
    if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) == -1) {
        perror("IOCTL failed to get flags");
        close(sockfd);
        return false;
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
        return false;
    }

    printf("Interface %s is now %s.\n", up ? "up" : "down", interface);

    // Close the socket
    close(sockfd);
    return true;
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

bool NetDevice::checkInterface() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return false;
    }

    struct ifreq ifr {};
    strncpy(ifr.ifr_name, interface, IFNAMSIZ);

    // Get interface flags (up/down)
    if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) == -1) {
        close(sockfd);
        return false;
    }

    bool is_up = ifr.ifr_flags & IFF_UP;
    bool is_running = ifr.ifr_flags & IFF_RUNNING;
    // std::cout << interface << " is " << (is_up ? "UP" : "DOWN") << " and " << (is_running ? "RUNNING" : "NOT RUNNING") << "\n";

    // Get wireless mode (managed/monitor/etc.)
    struct iwreq iwr {};
    strncpy(iwr.ifr_name, interface, IFNAMSIZ);
    if (ioctl(sockfd, SIOCGIWMODE, &iwr) == -1) {
        perror("SIOCGIWMODE (maybe not a wireless interface)");
    } else {
        /*
        std::cout << "Mode: ";
        switch (iwr.u.mode) {
            case IW_MODE_AUTO:    std::cout << "Auto"; break;
            case IW_MODE_ADHOC:   std::cout << "Ad-Hoc"; break;
            case IW_MODE_INFRA:   std::cout << "Managed"; break;
            case IW_MODE_MASTER:  std::cout << "Master"; break;
            case IW_MODE_REPEAT:  std::cout << "Repeater"; break;
            case IW_MODE_SECOND:  std::cout << "Secondary"; break;
            case IW_MODE_MONITOR: std::cout << "Monitor"; break;
            case IW_MODE_MESH:    std::cout << "Mesh"; break;
            default:              std::cout << "Unknown"; break;
        }
        std::cout << std::endl;
        */
    }

    close(sockfd);
    return true;
}
#endif
