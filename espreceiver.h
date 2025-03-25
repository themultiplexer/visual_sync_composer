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

struct thread_args {
    int sock_fd;
    void (*callback)(uint8_t src_mac[6], uint8_t *data, int len);
};

class espreceiver
{
public:
    espreceiver(std::string interface);
    void set_recv_callback(std::function<void (uint8_t[6], uint8_t *, int)>);
    void start();
    void unset_filter();
    void set_filter(uint8_t *dst_mac);

private:

    struct sock_fprog bpf;


    std::string interface;
    int sock_fd;
    std::thread thread;
    std::function<void(uint8_t[6], uint8_t *, int)> callback;
};

#endif // ESPRECEIVER_H
