
#ifndef MDNSFLASHER_H
#define MDNSFLASHER_H

#ifdef __APPLE__
#else
#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-common/simple-watch.h>
#include <avahi-common/error.h>
#endif

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
    mdnsflasher();
    static int flash(std::string firmware);

};

#endif // MDNSFLASHER_H
