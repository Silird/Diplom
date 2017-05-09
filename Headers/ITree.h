#ifndef DIPLOM_ITREE_H
#define DIPLOM_ITREE_H

class ITree {
public:
    virtual ~ITree(){};

    virtual void add(int value){};
    virtual bool search(int value){return false;};
    virtual void remove(int value){};
    virtual void print(){};
    virtual void printValues(){};
};

#endif //DIPLOM_ITREE_H
