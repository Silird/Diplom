#include <iostream>
#include "../Headers/TreeFactory.h"
#include "../Headers/BPlusTree/BPlusTree.h"

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
        tree = new BPlusTree(range);
    }
    return tree;
}

void TreeFactory::clear() {
    if (tree != nullptr) {
        delete tree;
        tree = nullptr;
    }

    if (instance != nullptr) {
        delete instance;
        instance = nullptr;
    }
}

