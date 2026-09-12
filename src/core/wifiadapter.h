#ifndef WIFIADAPTER_H
#define WIFIADAPTER_H

class WifiAdapter
{
public:
    WifiAdapter(const char *interface);

public:
    bool enableMonitorMode();
    bool setInterface(bool up);
    bool checkInterface();
private:
    const char *interface;
};

#endif // WIFIADAPTER_H
