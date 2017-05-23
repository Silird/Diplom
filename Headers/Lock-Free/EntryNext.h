#ifndef DIPLOM_ENTRYNEXT_H
#define DIPLOM_ENTRYNEXT_H

struct Entry;

//#pragma pack(push, 1)
struct EntryNext {
    bool freeze : 1;
    bool deletee : 1;
    Entry *next;

    bool operator == (const EntryNext &next) {
        return ((freeze == next.freeze) && (deletee == next.deletee) && (this->next == next.next));
    }

    bool operator != (const EntryNext &next) {
        return !((freeze == next.freeze) && (deletee == next.deletee) && (this->next == next.next));
    }

    /*
    EntryDataKey() noexcept : data(nullptr), key() {
    }

    EntryDataKey(unsigned short int state) noexcept : joinBuddy(nullptr), state(state) {
    }

    EntryDataKey(unsigned short int state, LockFreeElement *joinBuddy) noexcept :
            joinBuddy(joinBuddy), state(state)   {
    }
     */
};
//#pragma pack(pop)

#endif //DIPLOM_ENTRYNEXT_H
