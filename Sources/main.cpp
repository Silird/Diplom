#include <iostream>
#include "../Headers/TreeFactory.h"
#include "../Headers/Processing.h"
#include "../Headers/Lock-Free/NodeState.h"
#include "../Headers/Lock-Free/LockFreeElement.h"


int main(int argc, char * argv[]) {
    srand((unsigned int) time(0));

    if (argc < 3) {
        std::cout << "Use: ./Lab1 <N> <C> <R>, N - number of elements, C - number of cores, "
                "R - range of BTree" << std::endl;
        return EXIT_FAILURE;
    }

    int N = atoi(argv[1]); // Количество операций
    std::cout << "N = " << N << std::endl;
    int C = atoi(argv[2]); // Количество ядер
    std::cout << "C = " << C << std::endl;
    int R = atoi(argv[3]); // Range количество ссылок у B-дерева
    std::cout << "R = " << R << std::endl;


    Processing *processing;
    for (int cores = 1; cores <= C; cores++) {
        TreeFactory::getInstance(R)->getTree();

        processing = new Processing(N, cores);
        processing->startParallel();
        delete(processing);

        TreeFactory::getInstance()->clear();
    }


    /*
    ITree *tree = TreeFactory::getInstance(R)->getTree();


    tree->print();
    tree->printValues();

    std::cout << tree->search(10) << std::endl;

    tree->add(10);
    tree->print();

    std::cout << tree->search(10) << std::endl;

    tree->add(20);
    tree->print();

    tree->add(30);
    tree->print();

    tree->add(40);
    tree->print();

    tree->add(50);
    tree->print();

    tree->add(60);
    tree->print();

    tree->add(70);
    tree->print();

    tree->add(80);
    tree->print();


    std::cout << tree->search(60) << std::endl;
    std::cout << tree->search(65) << std::endl;

    tree->remove(80);
    tree->print();


    TreeFactory::getInstance()->clear();
    */

    return 0;
}



/*

ITree *tree = TreeFactory::getInstance(R)->getTree();
tree->print();

tree->add(0);

tree->print();

tree->remove(1);

tree->print();

tree->add(1);

tree->print();

tree->add(0);

tree->print();

tree->add(1);

tree->print();

tree->remove(1);

tree->print();

tree->add(0);

tree->print();

tree->remove(0);

tree->print();

tree->add(0);

tree->print();

tree->add(0);

tree->print();

tree->add(0);

tree->print();

tree->remove(1);

tree->print();

tree->add(1);

tree->print();

tree->add(1);

tree->print();

tree->add(1);

tree->print();

tree->add(0);

tree->print();

tree->remove(0);

tree->print();

tree->add(1);

tree->print();

tree->remove(0);

tree->print();

tree->remove(1);

tree->print();

tree->remove(0);

tree->print();

tree->add(1);

tree->print();

tree->remove(1);

tree->print();

tree->remove(1);


tree->print();
 */
/*
    ITree *tree = TreeFactory::getInstance(R)->getTree();

    tree->print();

    tree->add(10);

    tree->print();

    tree->add(20);

    tree->print();

    tree->add(30);

    tree->print();

    tree->add(40);

    tree->print();

    tree->add(50);

    tree->print();

    tree->add(60);

    tree->print();

    tree->add(70);

    tree->print();

    tree->add(80);

    tree->print();

    tree->add(90);

    tree->print();

    tree->printValues();

    //tree->remove(10);

    //tree->print();

    //tree->remove(10);
    //tree->remove(80);

    //tree->print();

    //tree->printValues();
*/


/*
LockFreeElement *node = new LockFreeElement(1);
node->height = 2;
node->counter = 10;

LockFreeElement *node1 = new LockFreeElement(1);
node1->height = 3;
node1->counter = 11;

std::atomic<NodeState> state;


NodeState tmp(JOIN, node);

tmp.joinBuddy = node;
tmp.state = JOIN;

state = tmp;

tmp = state;


NodeState stateTmp(JOIN, node1);
bool result;
result = state.compare_exchange_weak(stateTmp, NodeState(SLAVE_FREEZE, node1));
tmp = state;

stateTmp.state = SLAVE_FREEZE;
stateTmp.joinBuddy = node;
result = state.compare_exchange_weak(stateTmp, NodeState(SLAVE_FREEZE, node1));
tmp = state;

stateTmp.state = JOIN;
stateTmp.joinBuddy = node;
result = state.compare_exchange_weak(stateTmp, NodeState(SLAVE_FREEZE, node1));
tmp = state;


delete node;
 */