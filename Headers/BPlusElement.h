#ifndef DIPLOM_BPLUSELEMENT_H
#define DIPLOM_BPLUSELEMENT_H

struct BPlusElement {
    bool leaf;
    int range;

    int **data;
    BPlusElement **links;
    BPlusElement *parent;

    int distanceToDeep;
    static int deep;

    BPlusElement *left;
    BPlusElement *right;

    BPlusElement(int range) {
        init(range, nullptr, true);
    }

    BPlusElement(int range, bool leaf) {
        init(range, nullptr, leaf);
    }

    BPlusElement(int range, int data) {
        init(range, new int(data), true);
    }

    BPlusElement(int range, int data, bool leaf) {
        init(range, new int(data), leaf);
    }

    ~BPlusElement() {
        delete links;
        for (int i = 0; i < range - 1; i++) {
            delete data[i];
        }
        delete data;
    }

private:
    void init(int range, int *data, bool leaf) {
        this->range = range;
        this->data = new int*[range - 1];
        this->links = new BPlusElement*[range];
        for (int i = 0; i < range; i++) {
            links[i] = nullptr;
            if (i < range - 1) {
                this->data[i] = nullptr;
            }
        }
        this->data[0] = data;
        this->leaf = leaf;
        this->parent = nullptr;
        this->distanceToDeep = 0;
        this->left = nullptr;
        this->right = nullptr;
    }

public:
    void clearData() {
        for (int i = 0; i < range - 1; i++) {
            data[i] = nullptr;
        }
    }

    void addValue(int value) {
        int *newValue = new int(value);
        int *tmp = nullptr;
        for (int i = 0; i < range - 1; i++) {
            if (data[i] == nullptr) {
                if (newValue == nullptr) {
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
    }

    void addLink(BPlusElement *link) {
        int *newValue = new int(link->getLeftData());
        int place = 0;
        for (int i = 0; i < range - 1; i++) {
            if (data[i] == nullptr) {
                place = i;
                break;
            }
            else {
                if (*data[i] > *newValue) {
                    place = i;
                    break;
                }
            }
        }

        int *tmp = nullptr;
        for (int i = place; i < range - 1; i++) {
            if (data[i] == nullptr) {
                if (newValue == nullptr) {
                    break;
                }
                else {
                    data[i] = newValue;
                    newValue = nullptr;
                }
            }
            else {
                tmp = data[i];
                data[i] = newValue;
                newValue = tmp;
            }
        }

        BPlusElement *tmpLink = nullptr;
        for (int i = place + 1; i < range; i++) {
            if (links[i] == nullptr) {
                if (link == nullptr) {
                    break;
                }
                else {
                    links[i] = link;
                    link = nullptr;
                }
            }
            else {
                tmpLink = links[i];
                links[i] = link;
                link = tmpLink;
            }
        }
    }

    void refreshLink(BPlusElement *link) {
        int place = 0;

        for (int i = 0; i < range; i++) {
            if (links[i] == link) {
                place = i - 1;
                break;
            }
        }

        if (place >= 0) {
            int *tmp = data[place];
            data[place] = nullptr;
            delete tmp;
            tmp = new int(link->getLeftData());
            data[place] = tmp;
        }
    }

    void removeValue(int value) {
        int place = 0;
        for (int i = 0; i < range - 1; i++) {
            if (*data[i] == value) {
                place = i;
                break;
            }
        }

        delete data[place];
        data[place] = nullptr;

        for (int i = place; i < range - 2; i++) {
            if (data[i + 1] == nullptr) {
                break;
            }
            // TODO
            data[i] = data[i + 1];
        }
        data[range - 2] = nullptr;
    }

    int getDataSize() {
        int size = range - 1;
        for (int i = 0; i < range - 1; i++) {
            if (data[i] == nullptr) {
                size = i;
                break;
            }
        }

        return size;
    }

    int getLinkSize() {
        int size = range;
        for (int i = 0; i < range; i++) {
            if (links[i] == nullptr) {
                size = i;
                break;
            }
        }

        return size;
    }

    int getLinkPlace(BPlusElement *link) {
        int place = 0;
        for (int i = 0; i < range; i++) {
            if (links[i] == link) {
                place = i;
                break;
            }
        }

        return place;
    }

    void removeLink(BPlusElement *link) {
        int place = getLinkPlace(link);

        if (place - 1 >= 0) {
            delete data[place - 1];
            data[place - 1] = nullptr;
        }
        else if (data[place] != nullptr) {
            delete data[place];
            data[place] = nullptr;
        }

        for (int i = place - 1; i < range - 2; i++) {
            if (i >= 0) {
                if (data[i + 1] == nullptr) {
                    break;
                }
                // TODO
                data[i] = data[i + 1];
            }
        }

        links[place] = nullptr;

        for (int i = place; i < range - 1; i++) {
            if (links[i + 1] == nullptr) {
                break;
            }
            links[i] = links[i + 1];
        }

        links[range - 1] = nullptr;
        data[range - 2] = nullptr;
    }

    int getLeftData() {
        if (leaf) {
            /*
            if (data[0] == nullptr) {
                return -1;
            }
            else {
                return *data[0];
            }
             */

            return *data[0];
        }
        else {
            return links[0]->getLeftData();
        }
    }

    int getRightData() {
        if (leaf) {
            int place = 0;
            for (int i = 0; i < range - 1; i++) {
                if (data[i] == nullptr) {
                    place = i - 1;
                    break;
                }
            }

            return *data[place];
        }
        else {
            int place = 0;
            for (int i = 0; i < range; i++) {
                if (links[i] == nullptr) {
                    place = i - 1;
                    break;
                }
            }

            return links[place]->getRightData();
        }
    }
};

#endif //DIPLOM_BPLUSELEMENT_H
