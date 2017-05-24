#ifndef DIPLOM_FINDRESULT_H
#define DIPLOM_FINDRESULT_H

#include "LockFreeElement.h"

struct FindResult {
    std::atomic<EntryNext> *prev;
    EntryNext normalPrev;
    EntryNext cur;
    EntryNext next;
};

#endif //DIPLOM_FINDRESULT_H
