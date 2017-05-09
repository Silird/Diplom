#ifndef DIPLOM_PROCESSING_H
#define DIPLOM_PROCESSING_H

class Processing {
private:
    int N, C;
public:
    Processing(int N, int C);
    void start();
    void startParallel();
    static void *threadFunction(void *arg);
};

#endif //DIPLOM_PROCESSING_H
