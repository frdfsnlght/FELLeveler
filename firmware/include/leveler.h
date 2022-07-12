#ifndef LEVELER_H
#define LEVELER_H

#include "callback_list.h"

class Leveler {

    public:

    static Leveler* getInstance();

    CallbackList rollChangedListeners = CallbackList();
    CallbackList pitchChangedListeners = CallbackList();
    CallbackList implementAngleChangedListeners = CallbackList();

    int roll = 0;
    int pitch = 0;
    int implementAngle = 0;

    void setup();
    
    void calibrateLevel();
    void calibrateTipped();

    private:

    static Leveler* instance;

    Leveler() {}
    
    void update();
};

#endif
