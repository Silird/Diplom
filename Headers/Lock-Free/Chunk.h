#ifndef DIPLOM_CHUNK_H
#define DIPLOM_CHUNK_H

#include "Entry.h"

struct Chunk {
    Entry *head;

    Chunk() {
        head = new Entry();
    }

    ~Chunk() {
        Entry *tmp;
        while (head != nullptr) {
            tmp = head;
            head = head->next;
            delete tmp;
        }
    }
};

#endif //DIPLOM_CHUNK_H
