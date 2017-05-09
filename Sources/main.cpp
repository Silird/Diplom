#include <iostream>
#include "../Headers/TreeFactory.h"
#include "../Headers/Processing.h"

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
    //processing->startParallelStack();

    //TestFactory::getInstance()->clear();




/*
    for (int cores = 1; cores <= C; cores++) {
        TreeFactory::getInstance(R)->getTree();

        processing = new Processing(N, cores);
        processing->startParallel();
        delete(processing);

        TreeFactory::getInstance()->clear();
    }
*/


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

    return 0;
}