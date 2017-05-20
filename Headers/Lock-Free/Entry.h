#ifndef DIPLOM_ENTRIE_H
#define DIPLOM_ENTRIE_H

class LockFreeElement;

const short int INF = 32767;

struct Entry {
    short int key;

    LockFreeElement *data;

    /*
     * TODO
     * Эти переменные должны быть в одной как бы
     * 2 самых незначимых бита next должны быть фрозен и делитед
     */
    Entry *next;
    bool frozen;
    bool deleted;

    Entry() {
        key = INF;
        data = nullptr;
        next = nullptr;
        frozen = false;
        deleted = false;
    }
};

#endif //DIPLOM_ENTRIE_H
