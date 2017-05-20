#ifndef DIPLOM_LOCKFREEELEMENT_H
#define DIPLOM_LOCKFREEELEMENT_H

#include "Chunk.h"

// Freeze states
const short int INFANT = 0;
const short int NORMAL = 1;
const short int SLAVE_FREEZE = 2;
const short int FREEZE = 3;
const short int COPY = 4;
const short int SPLIT = 5;
const short int REQUEST_SLAVE = 6;
const short int JOIN = 7;

struct LockFreeElement {
    // хз зачем
    short int counter;
    int height = 0;

    // список записей, работа с которыми выполняется с помощью различных Lock-Free функций
    Chunk *chunk;

    // Ссылка на узел создатель этого
    LockFreeElement *creator;

    // Ссылки на новые узлы, которые надо вставить в дерево
    // nextNew содержится если узел содержится уже в nextNew
    LockFreeElement *neww;
    LockFreeElement *nextNew;

    /*
     * TODO
     * Переменные должны быть в 32 битах
     * freezeState должно занимать 3 LSB бита у джоинБадди
     */
    LockFreeElement *joinBuddy;
    short int freezeState;

    LockFreeElement() {
        counter = 0;
        height = 0;
        chunk = new Chunk();
        creator = nullptr;
        neww = nullptr;
        nextNew = nullptr;
        joinBuddy = nullptr;
        freezeState = INFANT;
    }

    ~LockFreeElement() {
        delete chunk;
    }
};

#endif //DIPLOM_LOCKFREEELEMENT_H