#ifndef DIPLOM_NODESTATE_H
#define DIPLOM_NODESTATE_H

struct LockFreeElement;

#pragma pack(push, 1)
struct NodeState {
    LockFreeElement *joinBuddy;
    unsigned short int state : 3;

    NodeState() noexcept : joinBuddy(nullptr), state(0) {
    }

    NodeState(short int state) noexcept : joinBuddy(nullptr), state(state) {
    }

    NodeState(short int state, LockFreeElement *joinBuddy) noexcept :
            joinBuddy(joinBuddy), state(state)   {
    }
};
#pragma pack(pop)

#endif //DIPLOM_NODESTATE_H
