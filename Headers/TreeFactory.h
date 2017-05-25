#ifndef DIPLOM_TREEFACTORY_H
#define DIPLOM_TREEFACTORY_H

#include "ITree.h"
#include "Lock-Free/LockFreeStack.h"

class TreeFactory {
private:
    static TreeFactory *instance;
    mutable int range = 3;
    ITree *tree = nullptr;
    LockFreeStack *stack = nullptr;

    TreeFactory(int range);
public:
    static TreeFactory* getInstance(int range);
    static TreeFactory* getInstance();
    ITree* getTree();
    LockFreeStack* getStack();
    void clear();
};

#endif //DIPLOM_TREEFACTORY_H
