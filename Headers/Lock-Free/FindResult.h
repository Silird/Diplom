#ifndef DIPLOM_FINDRESULT_H
#define DIPLOM_FINDRESULT_H

#include "LockFreeElement.h"

struct FindResult {
    Entry **prev;
    Entry *cur;
    Entry *next;
};

#endif //DIPLOM_FINDRESULT_H
