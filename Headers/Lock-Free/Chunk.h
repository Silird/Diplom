#ifndef DIPLOM_CHUNK_H
#define DIPLOM_CHUNK_H

#include "Entry.h"
#include <vector>

const unsigned short int IC_SUCCESS_THIS = 0;
const unsigned short int IC_SUCCESS_OTHER = 1;
const unsigned short int IC_EXISTED = 2;

struct Chunk {
    Entry *head;
    std::vector<Entry*> entries;

    Chunk(int range) {
        head = new Entry();
        for (int i = 0; i < range; i++) {
            Entry *tmp = new Entry();
            entries.push_back(tmp);
        }
    }

    ~Chunk() {
        Entry *tmp;
        for (int i = 0; i < entries.size(); i++) {
            delete entries[i];
            entries[i] = nullptr;
        }
    }
};

#endif //DIPLOM_CHUNK_H
