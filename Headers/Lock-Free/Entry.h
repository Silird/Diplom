#ifndef DIPLOM_ENTRIE_H
#define DIPLOM_ENTRIE_H

#include <atomic>

class LockFreeElement;

const short int INF = 32767;

struct Entry {
    short int key;

    std::atomic<LockFreeElement*> data;

    /*
     * TODO
     * Эти переменные должны быть в одной как бы
     * 2 самых незначимых бита next должны быть фрозен и делитед
     */
    std::atomic<Entry*> next;
    std::atomic<bool> frozen;
    std::atomic<bool> deleted;

    Entry() {
        key = INF;
        data = nullptr;
        next = nullptr;
        frozen = false;
        deleted = false;
        std::cout << "Entry created!" << std::endl;
    }

    ~Entry() {
        std::cout << "Entry deleted!" << std::endl;
    }
};

#endif //DIPLOM_ENTRIE_H
