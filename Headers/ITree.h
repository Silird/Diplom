#ifndef DIPLOM_ITREE_H
#define DIPLOM_ITREE_H

class ITree {
public:
    virtual ~ITree(){};

    virtual bool add(short int value){};
    virtual bool search(short int value){return false;};
    virtual bool remove(short int value){};
    virtual void print(){};
    virtual void printValues(){};
};

#endif //DIPLOM_ITREE_H
