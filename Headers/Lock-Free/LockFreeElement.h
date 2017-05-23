#ifndef DIPLOM_LOCKFREEELEMENT_H
#define DIPLOM_LOCKFREEELEMENT_H

#include "Chunk.h"
#include "NodeState.h"

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
    std::atomic<LockFreeElement*> creator;

    // Ссылки на новые узлы, которые надо вставить в дерево
    // nextNew содержится если узел содержится уже в nextNew
    std::atomic<LockFreeElement*> neww;
    std::atomic<LockFreeElement*> nextNew;

    // Состояние узла, состоит из ссылки на joinBuddy и кода состояния freezeState
    std::atomic<NodeState> state;

    LockFreeElement(int range) {
        counter = 0;
        height = 0;
        chunk = new Chunk(range);
        creator = nullptr;
        neww = nullptr;
        nextNew = nullptr;
        NodeState ns = state;
        ns.joinBuddy = nullptr;
        ns.freezeState = INFANT;
        std::cout << "LockFreeElement created!" << std::endl;
    }

    ~LockFreeElement() {
        delete chunk;
        std::cout << "LockFreeElement deleted!" << std::endl;
    }

    unsigned short int getFreezeState() {
        NodeState ns = state;
        return ns.freezeState;
    }

    LockFreeElement* getJoinBuddy() {
        NodeState ns = state;
        return ns.joinBuddy;
    }

    void setFreezeState(unsigned short int freezeState) {
        NodeState ns = state;
        ns.freezeState = freezeState;
        state = ns;
    }

    void setFreezeState(LockFreeElement *joinBuddy) {
        NodeState ns = state;
        ns.joinBuddy = joinBuddy;
        state = ns;
    }
};

#endif //DIPLOM_LOCKFREEELEMENT_H
