#ifndef LEVELER_H
#define LEVELER_H

#include <Arduino.h>

#include "callback_list.h"

class Leveler {

    public:

    static Leveler* getInstance();

    CallbackList anglesListeners = CallbackList();
    CallbackList remoteConnectedListeners = CallbackList();
    CallbackList remoteInfoListeners = CallbackList();
    CallbackList remoteAnglesListeners = CallbackList();

    int roll = 0;
    int pitch = 0;
    bool remoteConnected = false;
    String remoteName = "";
    String remoteAddress = "";
    int remoteRoll = 0;
    int remotePitch = 0;

    void setup();
    
    void calibrateLevel();
    void calibrateTipped();
    void setRemoteConnected(bool b);
    void setRemoteInfo(const String& name, const String& address);
    void setRemoteAngles(int roll, int pitch);

    private:

    static Leveler* instance;

    Leveler() {}
    
    void update();
};

#endif
