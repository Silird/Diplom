#ifndef DIPLOM_ENTRYDATAKEY_H
#define DIPLOM_ENTRYDATAKEY_H

struct LockFreeElement;

#pragma pack(push, 1)
struct EntryDataKey {
    short int key;
    bool freeze : 1;
    LockFreeElement *data;

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
#pragma pack(pop)

#endif //DIPLOM_ENTRYDATAKEY_H
