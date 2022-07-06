#ifndef LEVELER_H
#define LEVELER_H

#include "callback_list.h"

class Leveler {

    public:

    static Leveler* getInstance();

    CallbackList<int> listeners = CallbackList<int>();

    void setup();

    private:

    static Leveler* instance;

    static void accelerometerChanged(int);
    //static void bluetoothReceived(char* line);

    Leveler() {}
    
};

#endif
