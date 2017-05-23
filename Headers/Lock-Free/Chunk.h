#ifndef DIPLOM_CHUNK_H
#define DIPLOM_CHUNK_H

#include "Entry.h"
#include <vector>

const unsigned short int IC_SUCCESS_THIS = 0;
const unsigned short int IC_SUCCESS_OTHER = 1;
const unsigned short int IC_EXISTED = 2;

struct Chunk {
    std::atomic<EntryNext> head;
    std::vector<Entry*> entries;

    Chunk(int range) {
        EntryNext headTmp;
        headTmp.next = new Entry();
        headTmp.deletee = false;
        headTmp.freeze = false;
        head = headTmp;
        for (int i = 0; i < range; i++) {
            Entry *tmp = new Entry();
            entries.push_back(tmp);
        }
    }

    ~Chunk() {
        EntryNext headTmp = head;
        delete headTmp.next;
        Entry *tmp;
        for (int i = 0; i < entries.size(); i++) {
            delete entries[i];
            entries[i] = nullptr;
        }
    }

    Entry* getHead() {
        EntryNext headTmp = head;
        return headTmp.next;
    }
};

#endif //DIPLOM_CHUNK_H
