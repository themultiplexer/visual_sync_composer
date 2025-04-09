#ifndef MDNSFLASHER_H
#define MDNSFLASHER_H
#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-common/simple-watch.h>
#include <avahi-common/error.h>
#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-common/simple-watch.h>
#include <avahi-common/error.h>
#include <functional>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <string>

class mdnsflasher
{
public:
    mdnsflasher() : services(0) { };
    int flash(std::string firmware);

private:
    int services;
    void client_callback(AvahiClient *client, AvahiClientState state, void *userdata);
    void browse_callback(AvahiServiceBrowser *, AvahiIfIndex interface, AvahiProtocol protocol, AvahiBrowserEvent event, const char *name, const char *type, const char *domain, AvahiLookupResultFlags, void *userdata);
    void resolve_callback(AvahiServiceResolver *, AvahiIfIndex, AvahiProtocol, AvahiResolverEvent event, const char *name, const char *type, const char *domain, const char *host_name, const AvahiAddress *address, uint16_t port, AvahiStringList *, AvahiLookupResultFlags, void *userdata);
};

#endif // MDNSFLASHER_H
