#ifndef LEVELER_H
#define LEVELER_H

#include "callback_list.h"

class Leveler {

    public:

    static Leveler* getInstance();

    CallbackList rollChangedListeners = CallbackList();
    CallbackList pitchChangedListeners = CallbackList();
    CallbackList implementRollChangedListeners = CallbackList();
    CallbackList implementPitchChangedListeners = CallbackList();
    CallbackList implementLevelChangedListeners = CallbackList();

    int roll = 0;
    int pitch = 0;
    int implementRoll = 0;
    int implementPitch = 0;
    bool implementRollFrameLevel = false;
    bool implementRollEarthLevel = false;
    bool implementPitchFrameLevel = false;
    bool implementPitchEarthLevel = false;

    void setup();
    
    void calibrateLevel();
    void calibrateTipped();

    private:

    static Leveler* instance;

    Leveler() {}
    
    void update();
    void updateImplement();
    void updateLevel();
};

#endif
