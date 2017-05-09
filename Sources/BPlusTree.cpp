#include <iostream>
#include "../Headers/BPlusTree.h"

BPlusTree::BPlusTree(int range) {
    this->range = range;

    std::cout << "BPlusTree created!" << std::endl;
}

BPlusTree::~BPlusTree() {
    std::cout << "BPlusTree deleted!" << std::endl;
}

void BPlusTree::add(int value) {
    BPlusElement *insertNode = findElementToInsert(value);

    if (insertNode == nullptr) {
        root = new BPlusElement(range, value);
    }
    else {
        if (insertNode->getDataSize() + 1 < range) {
            insertNode->addValue(value);
        }
        else {
            split(insertNode, value, nullptr);
        }
    }
}

bool BPlusTree::search(int value) {
    BPlusElement *current = root;

    while (current != nullptr) {
        if (current->leaf) {
            for (int i = 0; i < range - 1; i++) {
                if (current->data[i] == nullptr) {
                    return false;
                }
                else {
                    if (*current->data[i] == value) {
                        return true;
                    }
                }
            }
            return false;
        }
        else {
            int next = range - 1;
            for (int i = 0; i < range - 1; i ++) {
                if (current->data[i] == nullptr) {
                    next = i;
                    break;
                }
                else {
                    if (*current->data[i] > value) {
                        next = i;
                        break;
                    }
                }
            }

            current = current->links[next];
        }
    }

    return false;
}

void BPlusTree::remove(int value) {
    BPlusElement *deleted = findElementToRemove(value);

    if (deleted != nullptr) {
        deleted->removeValue(value);

        /*
        if (deleted->parent != nullptr) {
            deleted->parent->refreshLink(deleted);
        }
         */

        if ((deleted != root) && (deleted->getDataSize() < (range)/2)) {
            BPlusElement *left = deleted->left;
            BPlusElement *right = deleted->right;
            if (left != nullptr) {
                if (left->getDataSize() > range/2) {
                    int place = range - 2;
                    for (int i = 0; i < range - 1; i++) {
                        if (left->data[i] == nullptr) {
                            place = i - 1;
                        }
                    }

                    deleted->addValue(*left->data[place]);
                    left->removeValue(*left->data[place]);

                    deleted->parent->refreshLink(deleted);
                    left->parent->refreshLink(left);
                }
                else {
                    if (right != nullptr) {
                        if (right->getDataSize() > range/2) {
                            int place = 0;
                            /*
                            for (int i = 0; i < range - 1; i++) {
                                if (right->data[i] == nullptr) {
                                    place = i - 1;
                                }
                            }
                             */

                            deleted->addValue(*right->data[place]);
                            right->removeValue(*right->data[place]);

                            deleted->parent->refreshLink(deleted);
                            right->parent->refreshLink(right);
                        }
                        else {
                            merge(deleted, right);
                        }
                    }
                    else {
                        merge(deleted, left);
                    }
                }
            }
            else {
                if (right->getDataSize() > range/2) {
                    int place = 0;
                    /*
                    for (int i = 0; i < range - 1; i++) {
                        if (right->data[i] == nullptr) {
                            place = i - 1;
                        }
                    }
                     */

                    deleted->addValue(*right->data[place]);
                    right->removeValue(*right->data[place]);

                    deleted->parent->refreshLink(deleted);
                    right->parent->refreshLink(right);
                }
                else {
                    merge(deleted, right);
                }
            }
        }
    }
}

void BPlusTree::print() {
    if (root == nullptr) {
        std::cout << "Tree is empty" << std::endl;
    }
    else {
        std::cout << "Tree:" << std::endl;
        print(root);
    }
}

void BPlusTree::printValues() {
    BPlusElement *node = root;

    if (node == nullptr) {
        std::cout << "Tree is empty" << std::endl;
    }
    else {
        while (!node->leaf) {
            node = node->links[0];
        }

        std::cout << "Tree:" << std::endl;

        while (node != nullptr) {
            for (int i = 0; i < range - 1; i++) {
                if (node->data[i] == nullptr) {
                    break;
                }
                else {
                    std::cout << *node->data[i] << " ";
                }
            }

            node = node->right;
        }

        std::cout << std::endl;
    }
}

void BPlusTree::print(BPlusElement *element) {
    if (element != nullptr) {
        for (int counter = 0; counter < element->range - 1; counter++) {
            if (element->data[counter] != nullptr) {
                for (int i = 0; i < BPlusElement::deep - element->distanceToDeep; i++) {
                    if (i == (element->deep - 1)) {
                        std::cout << "|--";
                    }
                    else {
                        std::cout << "|  ";
                    }
                }
                std::cout << *element->data[counter];
                if (element->leaf) {
                    std::cout << " (value)";
                }
                else {
                    std::cout << " (link)";
                }

                std::cout << std::endl;
            }
            else {
                break;
            }
        }

        std::cout << "|" << std::endl;

        for (int counter = 0; counter < element->range; counter++) {
            print(element->links[counter]);
        }
    }
}

BPlusElement* BPlusTree::findElementToInsert(int value) {
    BPlusElement *current = root;

    while (current != nullptr) {
        if (current->leaf) {
            return current;
        }
        else {
            int next = range - 1;
            for (int i = 0; i < range - 1; i ++) {
                if (current->data[i] == nullptr) {
                    next = i;
                    break;
                }
                else {
                    if (*current->data[i] > value) {
                        next = i;
                        break;
                    }
                }
            }

            current = current->links[next];
        }
    }

    return current;
}

void BPlusTree::split(BPlusElement *target, int value, BPlusElement *link) {
    BPlusElement *element2;

    if (target->leaf) {
        int **data = new int *[range];

        for (int i = 0; i < range - 1; i++) {
            if (target->data[i] == nullptr) {
                break;
            }
            data[i] = target->data[i];
            target->data[i] = nullptr;
        }

        int *newValue = new int(value);
        int *tmp = nullptr;

        int size = range;
        for (int i = 0; i < range; i++) {
            if (data[i] == nullptr) {
                if (newValue == nullptr) {
                    size = i;
                    break;
                }
                else {
                    data[i] = newValue;
                    newValue = nullptr;
                }
            }
            else {
                if (*data[i] > *newValue) {
                    tmp = data[i];
                    data[i] = newValue;
                    newValue = tmp;
                }
            }
        }

        int splitPoint = (size) / 2;
        int **data1 = new int *[range - 1];
        int **data2 = new int *[range - 1];

        for (int i = 0; i < range - 1; i++) {
            data1[i] = nullptr;
            data2[i] = nullptr;
        }

        for (int i = 0; i < range; i++) {
            if ((!target->leaf) && (i == splitPoint)) {
                continue;
            }
            if (i < splitPoint) {
                data1[i] = data[i];
            }
            else {
                data2[i - splitPoint - !target->leaf] = data[i];
            }
        }

        delete target->data;
        target->data = data1;

        element2 = new BPlusElement(range, target->leaf);
        element2->data = data2;
    }

    // ===============================

    else {
        BPlusElement **links = new BPlusElement *[range + 1];

        for (int i = 0; i < range; i++) {
            if (target->links[i] == nullptr) {
                break;
            }
            links[i] = target->links[i];
        }

        BPlusElement *newLink = link;
        BPlusElement *tmpLink = nullptr;

        int size = range + 1;
        for (int i = 0; i < range + 1; i++) {
            if (links[i] == nullptr) {
                if (newLink == nullptr) {
                    size = i;
                    break;
                }
                else {
                    links[i] = newLink;
                    newLink = nullptr;
                }
            }
            else {
                // Тут
                if ((newLink != nullptr) && (links[i]->getRightData() > newLink->getRightData())) {
                    tmpLink = links[i];
                    links[i] = newLink;
                    newLink = tmpLink;
                }
            }
        }

        int splitPoint = size/2;
        BPlusElement **link1 = new BPlusElement *[range];
        BPlusElement **link2 = new BPlusElement *[range];

        for (int i = 0; i < range; i++) {
            link1[i] = nullptr;
            link2[i] = nullptr;
        }

        for (int i = 0; i < range + 1; i++) {
            if (i < splitPoint) {
                link1[i] = links[i];
            }
            else {
                link2[i - splitPoint] = links[i];
            }
        }

        target->clearData();
        delete target->links;
        target->links = link1;
        for (int i = 0; i < range; i++) {
            if (target->links[i] == nullptr) {
                break;
            }
            else {
                target->links[i]->parent = target;
                target->refreshLink(target->links[i]);
            }
        }

        element2 = new BPlusElement(range, target->leaf);
        element2->links = link2;
        for (int i = 0; i < range; i++) {
            if (element2->links[i] == nullptr) {
                break;
            }
            else {
                element2->links[i]->parent = element2;
                element2->refreshLink(element2->links[i]);
            }
        }
    }

    // =========================================

    element2->parent = target->parent;
    element2->distanceToDeep = target->distanceToDeep;

    // Замена лефт райт

    element2->right = target->right;
    element2->left = target;
    target->right = element2;

    //



    BPlusElement *parent = target->parent;

    if (parent == nullptr) {
        root = new BPlusElement(range, false);
        parent = root;
        target->parent = parent;
        element2->parent = parent;
        parent->links[0] = target;
        BPlusElement::deep++;
        parent->distanceToDeep = target->distanceToDeep + 1;
    }
    else {
        // Тут
        parent->refreshLink(target);
    }

    addLinkToNode(parent, element2);
}

void BPlusTree::addLinkToNode(BPlusElement *target, BPlusElement *link) {
    if (target->getDataSize() + 1 < range) {
        target->addLink(link);
    }
    else {
        split(target, link->getLeftData(), link);
    }
}


BPlusElement* BPlusTree::findElementToRemove(int value) {
    BPlusElement *current = root;

    while (current != nullptr) {
        if (current->leaf) {
            for (int i = 0; i < range - 1; i++) {
                if (current->data[i] == nullptr) {
                    return nullptr;
                }
                else {
                    if (*current->data[i] == value) {
                        return current;
                    }
                }
            }
            return nullptr;
        }
        else {
            int next = range - 1;
            for (int i = 0; i < range - 1; i ++) {
                if (current->data[i] == nullptr) {
                    next = i;
                    break;
                }
                else {
                    if (*current->data[i] > value) {
                        next = i;
                        break;
                    }
                }
            }

            current = current->links[next];
        }
    }

    return nullptr;
}

void BPlusTree::merge(BPlusElement *node1, BPlusElement *node2) {
    BPlusElement *leftNode;
    BPlusElement *rightNode;

    //if (node1->getRightData() < node2->getRightData()) {
    if (node1->right == node2) {
        leftNode = node1;
        rightNode = node2;
    }
    else {
        leftNode = node2;
        rightNode = node1;
    }

    if (leftNode->leaf) {
        int place = 0;
        for (int i = 0; i < range - 1; i++) {
            if (leftNode->data[i] == nullptr) {
                place = i;
                break;
            }
        }

        for (int i = 0; i < range -1; i++) {
            if (rightNode->data[i] == nullptr) {
                break;
            }
            leftNode->data[place] = rightNode->data[i];
            rightNode->data[i] = nullptr;
            place++;
        }
    }
    else {
        int place = 0;
        for (int i = 0; i < range; i++) {
            if (leftNode->links[i] == nullptr) {
                place = i;
                break;
            }
        }

        for (int i = 0; i < range; i++) {
            if (rightNode->links[i] == nullptr) {
                break;
            }
            leftNode->links[place] = rightNode->links[i];
            leftNode->refreshLink(leftNode->links[place]);
            place++;
        }
    }

    rightNode->parent->removeLink(rightNode);

    leftNode->right = rightNode->right;

    delete rightNode;

    /*
    if (leftNode->parent != nullptr) {
        leftNode->parent->refreshLink(leftNode);
    }
     */

    BPlusElement *parent = leftNode->parent;
    if ((parent != root) && (parent->getLinkSize() < (range + 1)/2)) {
        BPlusElement *left = parent->left;
        BPlusElement *right = parent->right;

        if (left != nullptr) {
            if (left->getLinkSize() > (range + 1)/2) {
                int place = range - 1;
                for (int i = 0; i < range - 1; i++) {
                    if (left->links[i] == nullptr) {
                        place = i - 1;
                    }
                }

                parent->addLink(left->links[place]);
                left->removeLink(left->links[place]);
            }
            else {
                if (right != nullptr) {
                    if (right->getLinkSize() > (range + 1)/2) {
                        int place = 0;
                        /*
                        for (int i = 0; i < range - 1; i++) {
                            if (right->links[i] == nullptr) {
                                place = i - 1;
                            }
                        }
                         */

                        parent->addLink(right->links[place]);
                        right->removeLink(right->links[place]);
                    }
                    else {
                        merge(parent, right);
                    }
                }
                else {
                    merge(parent, left);
                }
            }
        }
        else {
            if (right->getLinkSize() > (range + 1)/2) {
                int place = 0;
                /*
                for (int i = 0; i < range - 1; i++) {
                    if (right->links[i] == nullptr) {
                        place = i - 1;
                    }
                }
                 */

                parent->addLink(right->links[place]);
                right->removeLink(right->links[place]);
            }
            else {
                merge(parent, right);
            }
        }
    }
    else if ((parent == root) && (parent->getDataSize() == 0)) {
        root = parent->links[0];
        root->parent = nullptr;
        delete parent;
        BPlusElement::deep--;
    }
}
