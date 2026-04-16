#ifndef DEVICEREQISTRY_H
#define DEVICEREQISTRY_H
#include <array>
#include <cstdint>
#include <vector>


class devicereqistry
{
public:
    devicereqistry();
    static std::vector<std::array<uint8_t, 6>> macs();
};

#endif // DEVICEREQISTRY_H
