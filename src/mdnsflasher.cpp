#include "mdnsflasher.h"


#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-common/simple-watch.h>
#include <avahi-common/error.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>

std::vector<std::string> found_hosts;
AvahiSimplePoll *simple_poll = nullptr;

int services = 0;
int serviceCount = 0;
int devices = 0;

void resolve_callback(
    AvahiServiceResolver *,
    AvahiIfIndex,
    AvahiProtocol,
    AvahiResolverEvent event,
    const char *name,
    const char *type,
    const char *domain,
    const char *host_name,
    const AvahiAddress *address,
    uint16_t port,
    AvahiStringList *,
    AvahiLookupResultFlags,
    void *userdata) {

    if (event == AVAHI_RESOLVER_FOUND) {
        char addr_str[AVAHI_ADDRESS_STR_MAX];
        avahi_address_snprint(addr_str, sizeof(addr_str), address);
        std::cout << "Found device: " << name << " at " << addr_str << std::endl;
        found_hosts.push_back(addr_str);
    } else {
        std::cerr << "Failed to resolve service '" << name << "': " << avahi_strerror(avahi_client_errno((AvahiClient *)userdata)) << std::endl;
    }

    devices++;

    if (devices == serviceCount) {
        avahi_simple_poll_quit(simple_poll);
    }
}

void browse_callback(
    AvahiServiceBrowser *,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char *name,
    const char *type,
    const char *domain,
    AvahiLookupResultFlags,
    void *userdata) {

    AvahiClient *client = static_cast<AvahiClient *>(userdata);

    switch (event) {
    case AVAHI_BROWSER_NEW:
        std::cout << "Service found: " << name << std::endl;
        avahi_service_resolver_new(client, interface, protocol, name, type, domain, AVAHI_PROTO_INET,
                                   AvahiLookupFlags::AVAHI_LOOKUP_USE_MULTICAST, resolve_callback, userdata);
        services++;
        break;

    case AVAHI_BROWSER_REMOVE:
        std::cout << "Service removed: " << name << std::endl;
        break;

    case AVAHI_BROWSER_ALL_FOR_NOW:
    case AVAHI_BROWSER_CACHE_EXHAUSTED:
        std::cout << "AVAHI_BROWSER_ALL_FOR_NOW " << std::endl;
        //avahi_simple_poll_quit(simple_poll);
        serviceCount = services;
        break;

    case AVAHI_BROWSER_FAILURE:
        std::cerr << "Browser failure: " << avahi_strerror(avahi_client_errno(client)) << std::endl;
        avahi_simple_poll_quit(simple_poll);
        break;
    }
}

static void client_callback(AvahiClient *client, AvahiClientState state, void *userdata) {
    if (state == AVAHI_CLIENT_FAILURE) {
        std::cerr << "Avahi client failure: " << avahi_strerror(avahi_client_errno(client)) << std::endl;
        avahi_simple_poll_quit(simple_poll);
    }

    services = 0;
}

int mdnsflasher::flash(std::string firmware)
{
    found_hosts.clear();
    services = 0;
    serviceCount = 0;
    devices = 0;
    int error;

    simple_poll = avahi_simple_poll_new();
    if (!simple_poll) {
        std::cerr << "Failed to create simple poll object.\n";
        return 1;
    }

    AvahiClient *client = avahi_client_new(avahi_simple_poll_get(simple_poll), AvahiClientFlags::AVAHI_CLIENT_IGNORE_USER_CONFIG, client_callback, nullptr, &error);
    if (!client) {
        std::cerr << "Failed to create client: " << avahi_strerror(error) << std::endl;
        return 1;
    }

    AvahiServiceBrowser *browser = avahi_service_browser_new(
        client, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC,
        "_arduino._tcp", "local", (AvahiLookupFlags)0,
        browse_callback, client);

    if (!browser) {
        std::cerr << "Failed to create service browser: " << avahi_strerror(avahi_client_errno(client)) << std::endl;
        return 1;
    }

    std::cout << "Searching for devices...\n";
    avahi_simple_poll_loop(simple_poll);

    std::cout << "Found " << found_hosts.size() << " devices.\n";
    for (const auto &ip : found_hosts) {
        std::string cmd = "python3 ../espota.py -p 8266 -i " + ip + " -f " + firmware;
        std::cout << "Flashing " << ip << "...\n";
        std::system(cmd.c_str());
    }

    return 0;
}
