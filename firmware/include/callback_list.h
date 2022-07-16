#ifndef CALLBACK_LIST_H
#define CALLBACK_LIST_H

class CallbackList {

    public:

    CallbackList() : first(nullptr), callDeferred(false) {}

    void add(void (*cb)(void));
    void remove(void (*cb)(void));
    void call();
    void callLater();
    void callNow();

    private:

    struct CallbackListNode {
        void (*cb)(void);
        CallbackListNode* next;
        CallbackListNode(void (*cb)(void)) : cb(cb), next(nullptr) {}
    };

    CallbackListNode* first;
    bool callDeferred;

};

#endif
