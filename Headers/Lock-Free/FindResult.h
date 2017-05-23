#ifndef DIPLOM_FINDRESULT_H
#define DIPLOM_FINDRESULT_H

#include "LockFreeElement.h"

struct FindResult {
    std::atomic<EntryNext> *prev;
    EntryNext cur;
    EntryNext next;
};

#endif //DIPLOM_FINDRESULT_H
