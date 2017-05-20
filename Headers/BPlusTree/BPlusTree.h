#ifndef DIPLOM_BPLUSTREE_H
#define DIPLOM_BPLUSTREE_H

#include "../ITree.h"
#include "BPlusElement.h"

class BPlusTree : public virtual ITree {
private:
    int range;
    BPlusElement *root;

public:
    BPlusTree(int range);
    ~BPlusTree();
    bool add(short int value);
    bool search(short int value);
    bool remove(short int value);
    void print();
    void printValues();

private:
    void print(BPlusElement *element);

    BPlusElement* findElementToInsert(short int value);
    void split(BPlusElement *target, short int value, BPlusElement *link);
    void addLinkToNode(BPlusElement *target, BPlusElement *link);

    BPlusElement* findElementToRemove(short int value);
    void merge(BPlusElement *node1, BPlusElement *node2);
};

#endif //DIPLOM_BPLUSTREE_H
