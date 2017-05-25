#include <iostream>
#include "../Headers/TreeFactory.h"
#include "../Headers/BPlusTree/BPlusTree.h"
#include "../Headers/Lock-Free/LockFreeTree.h"

TreeFactory *TreeFactory::instance = nullptr;

TreeFactory::TreeFactory(int range) {
    this->range = range;
}

TreeFactory* TreeFactory::getInstance(int range) {
    if (instance == nullptr) {
        instance = new TreeFactory(range);
    }
    return instance;
}

TreeFactory* TreeFactory::getInstance() {
    if (instance == nullptr) {
        std::cout << "Factory is not initialised yet!" << std::endl;
    }
    return instance;
}

ITree* TreeFactory::getTree() {
    if (tree == nullptr) {
        //tree = new BPlusTree(range);
        tree = new LockFreeTree(range);
    }
    return tree;
}

LockFreeStack* TreeFactory::getStack() {
    if (stack == nullptr) {
        stack = new LockFreeStack();
    }
    return stack;
}

void TreeFactory::clear() {
    if (tree != nullptr) {
        delete tree;
        tree = nullptr;
    }

    if (stack != nullptr) {
        delete stack;
        stack = nullptr;
    }

    if (instance != nullptr) {
        delete instance;
        instance = nullptr;
    }
}

