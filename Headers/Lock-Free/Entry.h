#ifndef DIPLOM_ENTRIE_H
#define DIPLOM_ENTRIE_H

#include <atomic>
#include "EntryDataKey.h"
#include "EntryNext.h"

class LockFreeElement;

const short int INF = 32767;
const short int EMPTY = 32766;

struct Entry {
    std::atomic<EntryDataKey> dataKey;

    std::atomic<EntryNext> next;

    Entry() {
        EntryDataKey dK = dataKey;
        dK.key = EMPTY;
        dK.data = nullptr;
        dK.freeze = false;
        EntryNext n = next;
        n.next = nullptr;
        n.deletee = false;
        n.freeze = false;
        std::cout << "Entry created!" << std::endl;
    }

    ~Entry() {
        std::cout << "Entry deleted!" << std::endl;
    }

    short int getKey() {
        EntryDataKey dK = dataKey;
        return dK.key;
    }

    LockFreeElement* getData() {
        EntryDataKey dK = dataKey;
        return dK.data;
    }

    Entry* getNext() {
        EntryNext n = next;
        return n.next;
    }

    void setKey(short int key) {
        EntryDataKey dK = dataKey;
        dK.key = key;
        dataKey = dK;
    }

    void setData(LockFreeElement *data) {
        EntryDataKey dK = dataKey;
        dK.data = data;
        dataKey = dK;
    }

    void setNext(Entry *next) {
        EntryNext n = this->next;
        n.next = next;
        this->next = n;
    }
};

#endif //DIPLOM_ENTRIE_H
