#ifndef CALLBACK_LIST_H
#define CALLBACK_LIST_H

class CallbackList {

    public:

    CallbackList() : first(nullptr), callDeferred(false) {}

    void add(void (*cb)(void)) {
        if (first == nullptr)
            first = new CallbackListNode(cb);
        else {
            CallbackListNode* node = first;
            while (node->next != nullptr)
                node = node->next;
            node->next = new CallbackListNode(cb);
        }
    }

    void remove(void (*cb)(void)) {
        CallbackListNode* node = first;
        CallbackListNode* last = nullptr;
        while (node != nullptr) {
            if (node->cb == cb) {
                if (last == nullptr)
                    first = node->next;
                else
                    last->next = node->next;
                delete node;
                return;
            }
            last = node;
            node = node->next;
        }
    }

    void call() {
        CallbackListNode* node = first;
        while (node != nullptr) {
            node->cb();
            node = node->next;
        }
    }

    void callLater() {
        callDeferred = true;
    }

    void callNow() {
        if (callDeferred) {
            callDeferred = false;
            call();
        }
    }

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
