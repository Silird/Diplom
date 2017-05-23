#ifndef DIPLOM_NODESTATE_H
#define DIPLOM_NODESTATE_H

struct LockFreeElement;

#pragma pack(push, 1)
struct NodeState {
    LockFreeElement *joinBuddy;
    unsigned short int freezeState : 3;

    NodeState() noexcept : joinBuddy(nullptr), freezeState(0) {
    }

    NodeState(unsigned short int freezeState) noexcept : joinBuddy(nullptr), freezeState(freezeState) {
    }

    NodeState(unsigned short int freezeState, LockFreeElement *joinBuddy) noexcept :
            joinBuddy(joinBuddy), freezeState(freezeState)   {
    }
};
#pragma pack(pop)

#endif //DIPLOM_NODESTATE_H
