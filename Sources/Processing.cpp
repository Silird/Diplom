#include <cstdlib>
#include <iostream>
#include <sys/time.h>

#include "../Headers/Processing.h"
#include "../Headers/TreeFactory.h"
#include "../Headers/Lock-Free/LockFreeStack.h"

using  namespace std;

Processing::Processing(int N, int C) {
    this->N = N;
    this->C = C;
}

void Processing::start() {
    ITree *tree = TreeFactory::getInstance()->getTree();

    short int limit = 10000;
    for (int i = 0; i < N/C; i++) {
        //std::cout << "Operation " << i << std::endl;
        switch (rand() % 7) {
            case 0: {
            }
            case 1: {
                short int number = rand() % limit;
                //std::cout << "tree->remove(" << number << "); //" << i << std::endl;
                tree->remove(number);
                break;

            }
            case 2: {
            }
            case 3: {
                short int number = rand() % limit;
                //std::cout << "(" << i << ") Search " << number << std::endl;
                tree->search(number);
                break;
            }
            default: {
                short int number = rand() % limit;
                //std::cout << "tree->add(" << number << "); //" << i << std::endl;
                tree->add(number);
                break;
            }
        }
    }
}

void Processing::startParallel() {
    struct timeval startTime, finishTime, overallTime;
    struct timezone tz;

    gettimeofday(&startTime, &tz);

    pthread_t threads[C];
    if (C == 1) {
        start();
    }
    else {
        for (int i = 0; i < C; i++) {
            if (pthread_create(&threads[i], NULL, threadFunction, this)) {
                cout << "Error pthread_create of thread " << i << endl;
            }
        }

        for (int i = 0; i < C; i++) {
            if (int result = pthread_join(threads[i], NULL) != 0) { // Ожидает завершения потока
                cout << "Error pthread_join of thread " << i << ": " << result << endl;
            }
        }
    }

    gettimeofday(&finishTime, &tz);

    overallTime.tv_sec = finishTime.tv_sec - startTime.tv_sec;

    overallTime.tv_usec = finishTime.tv_usec - startTime.tv_usec;

    if (overallTime.tv_usec < 0) {
        overallTime.tv_sec--;
        overallTime.tv_usec += 1000000;
    }

    double doubleTime = overallTime.tv_sec*1000 + overallTime.tv_usec/1000;
    //cout << "Finish time: " << finishTime << endl;
    //double countTime = difftime(finishTime, startTime);

    cout << "Threads: " << C << ", Elements: " << N << endl;
    cout << "Time: " << doubleTime << endl << endl;
}

void *Processing::threadFunction(void *arg) {
    Processing *processing = (Processing*) arg;
    processing->start();
    return nullptr;
}