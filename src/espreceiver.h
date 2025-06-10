#ifndef ESPRECEIVER_H
#define ESPRECEIVER_H
#include <cstdint>
#include <functional>
#include <linux/filter.h>
#include <string>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if_arp.h>
#include <arpa/inet.h>
#include <cstdint>
#include <linux/if_ether.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <thread>
#include <span>

class espreceiver
{
public:
    espreceiver(std::string interface);
    void set_recv_callback(std::function<void (std::array<uint8_t, 6>, std::span<uint8_t>)>);
    void start();
    void unset_filter();
    void set_filter(std::array<uint8_t, 6> * dst_mac);

private:
    struct sock_fprog bpf;

    std::string interface;
    int sock_fd;
    std::thread thread;
    std::function<void(std::array<uint8_t, 6>, std::span<uint8_t>)> callback;
};

#endif // ESPRECEIVER_H
