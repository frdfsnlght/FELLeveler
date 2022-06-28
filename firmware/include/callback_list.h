#ifndef CALLBACK_LIST_H
#define CALLBACK_LIST_H

template <class T>
class CallbackList {

    public:

    CallbackList() : first(nullptr) {}

    void add(void (*cb)(T)) {
        if (first == nullptr)
            first = new CallbackListNode(cb);
        else {
            CallbackListNode* node = first;
            while (node->next != nullptr)
                node = node->next;
            node->next = new CallbackListNode(cb);
        }
    }

    void remove(void (*cb)(T)) {
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

    void call(T arg) {
        CallbackListNode* node = first;
        while (node != nullptr) {
            node->cb(arg);
            node = node->next;
        }
    }
    
    private:

    struct CallbackListNode {
        void (*cb)(T);
        CallbackListNode* next;
        CallbackListNode(void (*cb)(T)) : cb(cb), next(nullptr) {}
    };

    CallbackListNode* first;

};

#endif
