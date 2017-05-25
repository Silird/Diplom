#ifndef DIPLOM_LOCKFREESTACK_H
#define DIPLOM_LOCKFREESTACK_H

#include <atomic>
#include "LockFreeStackElement.h"

class LockFreeStack {
private:
    std::atomic<LockFreeStackElement*> head;
    std::atomic<unsigned> threads_in_pop;
    std::atomic<LockFreeStackElement*> delete_list;

public:
    LockFreeStack() {
        head = nullptr;
        threads_in_pop = 0;
        delete_list = nullptr;
    }

    ~LockFreeStack() {
        auto first = head.load(std::memory_order_relaxed);
        while (first) {
            auto unlinked = first;
            first = first->next;
            delete unlinked;
        }
    }

    void push(LockFreeElement* data) {
        LockFreeStackElement *new_node = new LockFreeStackElement(data);
        new_node->next = head.load();
        while (!head.compare_exchange_weak(new_node->next, new_node));
    }

    LockFreeElement* pop() {
        threads_in_pop++;
        LockFreeStackElement* old_head = head.load();
        while (old_head &&
               !head.compare_exchange_weak(old_head,
                                           old_head->next));
        LockFreeElement* res = nullptr;
        if (old_head) {
            res = old_head->data;
        }

        try_reclaim(old_head);

        return res;
    }

private:
    static void delete_nodes(LockFreeStackElement *nodes) {
        while (nodes) {
            LockFreeStackElement *next = nodes->next;
            delete nodes;
            nodes = next;
        }
    }

    void try_reclaim(LockFreeStackElement *old_head) {
        if (threads_in_pop == 1) {
            LockFreeStackElement *nodes_to_delete = delete_list.exchange(nullptr);
            if (!--threads_in_pop) {
                delete_nodes(nodes_to_delete);
            }
            else if (nodes_to_delete) {
                chain_pending_nodes(nodes_to_delete);
            }
            delete old_head;
        }
        else {
            chain_pending_node(old_head);
            --threads_in_pop;
        }
    }

    void chain_pending_nodes(LockFreeStackElement *nodes) {
        LockFreeStackElement *last = nodes;
        while (LockFreeStackElement *const next = last->next) {
            last = next;
        }
        chain_pending_nodes(nodes, last);
    }

    void chain_pending_nodes(LockFreeStackElement *first, LockFreeStackElement *last) {
        last->next = delete_list;
        while (!delete_list.compare_exchange_weak(last->next, first));
    }

    void chain_pending_node(LockFreeStackElement* n) {
        chain_pending_nodes(n, n);
    }

};
#endif //DIPLOM_LOCKFREESTACK_H
