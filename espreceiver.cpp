#include "espreceiver.h"
#include "espnowtypes.h"
#include <cassert>

void espreceiver::set_recv_callback(std::function<void (uint8_t[6], uint8_t *, int)> recv_callback) {
    callback = recv_callback;
}

espreceiver::espreceiver(std::string interface) {
    this->interface = interface;
    this->bpf.len = 0;
}

void espreceiver::unset_filter() {
    //thread.join();
    if(this->bpf.filter != NULL) {
        free(this->bpf.filter);
        this->bpf.filter = NULL;
    }
    this->bpf.len = 0;
}

void espreceiver::set_filter(uint8_t *dst_mac) {
    //sudo tcpdump -i wlp5s0 'type 0 subtype 0xd0 and wlan[24:4]=0x7f18fe34 and wlan[32]=221 and wlan[33:4]&0xffffff = 0x18fe34 and wlan[37]=0x4 and wlan dst 11:22:33:44:55:66' -dd
    //unset_filter();

    this->bpf.len = 34;

    uint32_t MSB_dst = MAC_2_MSBytes(dst_mac);
    uint32_t LSB_dst = MAC_4_LSBytes(dst_mac);

    uint8_t jeq_dst = dst_mac == NULL ? 0x30 : 0x15; //0x30 jump if >=. 0x15 jump if ==.

    struct sock_filter temp_code[34] = {
        { 0x30, 0, 0, 0x00000003 },
        { 0x64, 0, 0, 0x00000008 },
        { 0x7, 0, 0, 0x00000000 },
        { 0x30, 0, 0, 0x00000002 },
        { 0x4c, 0, 0, 0x00000000 },
        { 0x2, 0, 0, 0x00000000 },
        { 0x7, 0, 0, 0x00000000 },
        { 0x50, 0, 0, 0x00000000 },
        { 0x54, 0, 0, 0x000000fc },
        { 0x15, 0, 23, 0x000000d0 },
        { 0x40, 0, 0, 0x00000018 },
        { 0x15, 0, 21, 0x7f18fe34 },
        { 0x50, 0, 0, 0x00000020 },
        { 0x15, 0, 19, 0x000000dd },
        { 0x40, 0, 0, 0x00000021 },
        { 0x54, 0, 0, 0x00ffffff },
        { 0x15, 0, 16, 0x0018fe34 },
        { 0x50, 0, 0, 0x00000025 },
        { 0x15, 0, 14, 0x00000004 },
        { 0x50, 0, 0, 0x00000000 },
        { 0x45, 12, 0, 0x00000004 },
        { 0x45, 0, 6, 0x00000008 },
        { 0x50, 0, 0, 0x00000001 },
        { 0x45, 0, 4, 0x00000001 },
        { 0x40, 0, 0, 0x00000012 },
        { 0x15, 0, 7, LSB_dst },
        { 0x48, 0, 0, 0x00000010 },
        { 0x15, 4, 5, MSB_dst },
        { 0x40, 0, 0, 0x00000006 },
        { 0x15, 0, 3, LSB_dst },
        { 0x48, 0, 0, 0x00000004 },
        { 0x15, 0, 1, MSB_dst },
        { 0x6, 0, 0, 0x00040000 },
        { 0x6, 0, 0, 0x00000000 },
    };

    this->bpf.filter = (sock_filter*) malloc(sizeof(sock_filter)*this->bpf.len);
    memcpy(this->bpf.filter, temp_code, sizeof(struct sock_filter) * this->bpf.len);
}


void espreceiver::start() {
    struct sockaddr_ll s_dest_addr;
    struct ifreq ifr;

    int fd, 			//file descriptor
        ioctl_errno,	//ioctl errno
        bind_errno,		//bind errno
        priority_errno;	//Set priority errno

    bzero(&s_dest_addr, sizeof(s_dest_addr));
    bzero(&ifr, sizeof(ifr));

    fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    assert(fd != -1);

    strncpy((char *)ifr.ifr_name, this->interface.c_str(), IFNAMSIZ); //interface

    ioctl_errno = ioctl(fd, SIOCGIFINDEX, &ifr);
    assert(ioctl_errno >= 0);	//abort if error

    s_dest_addr.sll_family = PF_PACKET;
    s_dest_addr.sll_protocol = htons(ETH_P_ALL);
    s_dest_addr.sll_ifindex = ifr.ifr_ifindex;

    bind_errno = bind(fd, (struct sockaddr *)&s_dest_addr, sizeof(s_dest_addr));

    if(bpf.len > 0) {
        if (setsockopt(fd, SOL_SOCKET, SO_ATTACH_FILTER, &bpf, sizeof(bpf)) == -1) {
            perror("setsockopt");
        }
    }

    this->sock_fd = fd;

    std::vector<uint8_t> old_raw_bytes;

    // Constructs the new thread and runs it. Does not block execution.
    thread = std::thread([this, &old_raw_bytes](){
        int raw_bytes_len;
        uint8_t raw_bytes[LEN_RAWBYTES_MAX];

        uint8_t* res_mac;
        uint8_t* res_payload;
        int res_len;


        if(callback == NULL) {
            //printf ("No callback for receive, receive thread exited\n");
            return EXIT_SUCCESS;
        };

        while(1)
        {
            raw_bytes_len = recvfrom(sock_fd, raw_bytes, LEN_RAWBYTES_MAX, MSG_TRUNC, NULL, 0);

            if( -1 == raw_bytes_len )
            {
                perror ("Socket receive failed");
                break;
            }
            else if( raw_bytes_len < 0 )
            {
                perror ("Socket receive, error ");
            }
            else
            {
                //old_raw_bytes = std::vector<uint8_t>(raw_bytes, raw_bytes + raw_bytes_len);

                res_mac = ESPNOW_packet::get_src_mac(raw_bytes,raw_bytes_len);
                res_payload = ESPNOW_packet::get_payload(raw_bytes, raw_bytes_len);
                res_len = ESPNOW_packet::get_payload_len(raw_bytes, raw_bytes_len);
                if(res_mac != NULL && res_payload != NULL && res_len > 0) {
                    callback(res_mac, res_payload, res_len);
                }
            }
        }

        printf ("Receive thread exited \n");
        return EXIT_SUCCESS;
    });

}



