#ifndef DIPLOM_LOCKFREESTACKELEMENT_H
#define DIPLOM_LOCKFREESTACKELEMENT_H

struct LockFreeElement;

struct LockFreeStackElement {
    LockFreeElement* data;
    LockFreeStackElement *next;

    LockFreeStackElement(LockFreeElement* data) : data(data) {};
};
#endif //DIPLOM_LOCKFREESTACKELEMENT_H
