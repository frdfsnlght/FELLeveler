#ifndef BTDEVICE_H
#define BTDEVICE_H

struct BTDevice {

    static const int MaxBTNameLength = 32;
    static const int MaxBTAddressLength = 18;

    char name[MaxBTNameLength];
    char address[MaxBTAddressLength];

};

#endif
