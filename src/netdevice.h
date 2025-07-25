#ifndef NETDEVICE_H
#define NETDEVICE_H

class NetDevice
{
public:
    NetDevice(const char *interface);

public:
    bool enableMonitorMode();
    bool setInterface(bool up);
    bool checkInterface();
private:
    const char *interface;
};

#endif // NETDEVICE_H
