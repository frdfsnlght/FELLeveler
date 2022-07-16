#include "callback_list.h"

void CallbackList::add(void (*cb)(void)) {
    if (first == nullptr)
        first = new CallbackListNode(cb);
    else {
        CallbackListNode* node = first;
        while (node->next != nullptr)
            node = node->next;
        node->next = new CallbackListNode(cb);
    }
}

void CallbackList::remove(void (*cb)(void)) {
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

void CallbackList::call() {
    CallbackListNode* node = first;
    while (node != nullptr) {
        node->cb();
        node = node->next;
    }
}

void CallbackList::callLater() {
    callDeferred = true;
}

void CallbackList::callNow() {
    if (callDeferred) {
        callDeferred = false;
        call();
    }
}

