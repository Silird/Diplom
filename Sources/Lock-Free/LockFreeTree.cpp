#include <iostream>
#include "../../Headers/Lock-Free/LockFreeTree.h"

LockFreeTree::LockFreeTree(int range) {
    MAX = range;
    MIN = range/2 - 3;
    root = Allocate();
    root.load(std::memory_order_relaxed)->freezeState = NORMAL;

    /*
    Entry *entry = new Entry();
    entry->key = 10;
    Entry *master = root.load(std::memory_order_relaxed)->chunk->head;
    Entry *tmp = nullptr;
    master->next.compare_exchange_weak(tmp, entry);
     */

    std::cout << "LockFreeTree created!" << std::endl;
}

LockFreeTree::~LockFreeTree() {
    for (int i = 0; i < nodes.size(); i++) {
        delete nodes[i];
    }
    std::cout << "LockFreeTree deleted!" << std::endl;
}

bool LockFreeTree::add(short int key) {
    LockFreeElement *node = FindLeaf(key);
    if (node->freezeState == INFANT) {
        helpInfant(node);
    }
    return InsertToChunk(node, key, nullptr);
}

bool LockFreeTree::search(short int key) {
    LockFreeElement *node = FindLeaf(key);
    return SearchInChunk(node->chunk, key);
}

bool LockFreeTree::remove(short int key) {
    LockFreeElement *node = FindLeaf(key);
    if (node->freezeState == INFANT) {
        helpInfant(node);
    }
    return DeleteInChunk(node, key);
}

void LockFreeTree::print() {
    LockFreeElement *tmpRoot = root;
    if (tmpRoot->chunk->head->next == nullptr) {
        std::cout << "Tree is empty" << std::endl;
    }
    else {
        std::cout << "Tree:" << std::endl;
        print(tmpRoot, tmpRoot->height);
    }
}

void LockFreeTree::printValues() {
    LockFreeElement *tmpRoot = root;
    if (tmpRoot->chunk->head->next == nullptr) {
        std::cout << "Tree is empty" << std::endl;
    }
    else {
        std::cout << "Tree:" << std::endl;
        printValues(tmpRoot);
        std::cout << std::endl;
    }
}

void LockFreeTree::print(LockFreeElement *element, int maxHeight) {
    if (element != nullptr) {
        Entry *current = element->chunk->head->next;
        while (current != nullptr) {
            for (int i = 0; i < maxHeight - element->height; i++) {
                if (i == (element->height - 1)) {
                    std::cout << "|--";
                }
                else {
                    std::cout << "|  ";
                }
            }
            std::cout << current->key;
            if (element->height == 0) {
                std::cout << " (value)";
            }
            else {
                std::cout << " (link)";
            }

            std::cout << std::endl;

            current = current->next;
        }

        std::cout << "|" << std::endl;

        current = element->chunk->head->next;
        while (current != nullptr) {
            print(current->data, maxHeight);

            current = current->next;
        }
    }
}

void LockFreeTree::printValues(LockFreeElement *element) {
    if (element != nullptr) {
        if (element->height != 0) {
            Entry *current = element->chunk->head->next;
            while (current != nullptr) {
                printValues(current->data);

                current = current->next;
            }
        }
        else {
            Entry *current = element->chunk->head->next;
            while (current != nullptr) {
                std::cout << current->key << " ";

                current = current->next;
            }
        }
    }
}


// те, которые известны

/*
 * Находит лист, у которого ПОТЕНЦИАЛЬНО может находится данный ключ
 */
LockFreeElement* LockFreeTree::FindLeaf(short int key) {
    LockFreeElement *node = root;
    while ( node->height != 0 ) {
        FindResult findResult;
        Find(node->chunk, key, &findResult);
        node = findResult.cur->data;
    }
    return node;
}

/*
 * Находит родительский узел, в котором содержится пара key и childNode
 * Записвает в prntEnt запись целевого узла
 * Если slaveEnt не нул, то записывает в него потенциального раба для мерджа
 */
LockFreeElement* LockFreeTree::FindParent(short int key, LockFreeElement *childNode,
                            Entry **prntEnt, Entry **slaveEnt) {
    LockFreeElement *node = root;
    while (node->height != 0) {
        FindResult findResult;
        Find(node->chunk, key, &findResult);
        if (childNode == findResult.cur->data) {
            *prntEnt = findResult.cur;
            if (slaveEnt != NULL) {
                if (findResult.prev == &(node->chunk->head)) {
                    *slaveEnt = findResult.next;
                }
                else {
                    *slaveEnt = EntPtr(findResult.prev);
                }
            }
            if (node->freezeState == INFANT) {
                helpInfant(node);
            }
            return node;
        }
        node = findResult.cur->data;
    }
    return nullptr;
}

/*
 * Вставляет в дерево вместо узла node, два разделённых узла
 * Первый узел содержится в поле neww у node, а второй в поле nextNew у neww
 * Первый содержит меньшую часть ключей, второй большую
 * sepKey - ключ для второго узла.
 */
void LockFreeTree::InsertSplitNodes(LockFreeElement *node, short int sepKey) {
    Entry *nodeEnt;
    LockFreeElement *n1 = node->neww;
    LockFreeElement *n2 = node->neww.load(std::memory_order_relaxed)->nextNew;
    short int maxKey = getMaxKey(node);
    LockFreeElement *parent;
    if ((parent = FindParent(sepKey, node, &nodeEnt, nullptr)) != nullptr) {
        InsertToChunk(parent, sepKey, n1);
    }
    if ((parent = FindParent(maxKey, node, &nodeEnt, nullptr)) != nullptr) {
        ReplaceInChunk(parent, nodeEnt->key,
                combine(nodeEnt->key, node), combine(nodeEnt->key, n2));
    }
    // TODO нормальные касы
    short int tmp;
    LockFreeElement *tmpEl;
    if ((n1->freezeState == INFANT) && (n1->joinBuddy == nullptr)) {
        tmp = INFANT;
        tmpEl = nullptr;
        n1->freezeState.compare_exchange_weak(tmp, NORMAL);
        n1->joinBuddy.compare_exchange_weak(tmpEl, nullptr);
    }
    if ((n2->freezeState == INFANT) && (n2->joinBuddy == nullptr)) {
        tmp = INFANT;
        tmpEl = nullptr;
        n2->freezeState.compare_exchange_weak(tmp, NORMAL);
        n2->joinBuddy.compare_exchange_weak(tmpEl, nullptr);
    }
    return;
}

/*
 * Находит раба для мёрджа с master
 * Устанавливет состояния у обоих узлов
 */
LockFreeElement* LockFreeTree::FindJoinSlave(LockFreeElement *master, LockFreeElement *oldSlave) {
    short int anyKey = master->chunk->head->next.load(std::memory_order_relaxed)->key;
    LockFreeElement *parent;
    Entry *masterEnt;
    Entry *slaveEnt;
    if ((parent = FindParent(anyKey, master, &masterEnt, &slaveEnt)) == nullptr) {
        // TODO возвратить джоин бади, без всяких LSB bit
        return master->joinBuddy;
    }
    LockFreeElement *slave = slaveEnt->data;

    // TODO сделать нормальный объединённое состояние
    short int expState;
    LockFreeElement *expJoinBuddy;
    if (oldSlave == nullptr) {
        // TODO сделать нормальный объединённое состояние
        expState = FREEZE;
        expJoinBuddy = nullptr;
    }
    else  {
        // TODO сделать нормальный объединённое состояние
        expState = REQUEST_SLAVE;
        expJoinBuddy = oldSlave;
    }

    // TODO сделать нормальный кас
    bool casResult = false;
    if ((master->freezeState == expState) && (master->joinBuddy == expJoinBuddy)) {
        casResult = master->freezeState.compare_exchange_weak(expState, REQUEST_SLAVE);
        casResult = casResult && master->joinBuddy.compare_exchange_weak(expJoinBuddy, slave);
    }
    if (!casResult) {
        // TODO сделать нормальные сравнения с объединёнными переменными
        if (master->freezeState == JOIN) {
            return master->joinBuddy;
        }
    }
    // TODO сделать нормальные сравнения с объединёнными переменными
    slave = master->joinBuddy;
    // TODO сделать нормальные сравнения с объединёнными переменными
    if ((parent->freezeState != NORMAL) && (oldSlave == nullptr)) {
        bool result;
        Freeze(parent, 0, nullptr, master, TT_NONE, &result);
        FindJoinSlave(master, slave);
    }
    if (!SetSlave(master, slave, anyKey, slave->chunk->head->next.load(std::memory_order_relaxed)->key)) {
        FindJoinSlave(master, slave);
    }

    // TODO сделать нормальный кас
    expState = REQUEST_SLAVE;
    expJoinBuddy = slave;
    if ((master->freezeState == expState) && (master->joinBuddy == expJoinBuddy)) {
        master->freezeState.compare_exchange_weak(expState, JOIN);
        master->joinBuddy.compare_exchange_weak(expJoinBuddy, slave);
    }

    // TODO сделать нормальные сравнения с объединёнными переменными
    if (master->freezeState == JOIN) {
        return slave;
    }

    return nullptr;
}

/*
 * Пытается установить отношение master-slave
 * Может произойти смена ролей
 */
bool LockFreeTree::SetSlave(LockFreeElement *master, LockFreeElement *slave, short int masterKey,
              short int slaveKey) {
    short int expState = NORMAL;
    LockFreeElement *expJoinBuddy = nullptr;
    // TODO сделать нормальный кас в вайл вставить
    bool casResult = false;
    if ((master->freezeState == expState) && (master->joinBuddy == expJoinBuddy)) {
        casResult = master->freezeState.compare_exchange_weak(expState, SLAVE_FREEZE);
        casResult = casResult && master->joinBuddy.compare_exchange_weak(expJoinBuddy, master);
    }


    if (!casResult) {
        // TODO сделать нормальные сравнения с объединёнными переменными
        if (slave->freezeState == INFANT) {
            helpInfant(slave);
            return false;
        }
        // TODO сделать нормальные сравнения с объединёнными переменными
        else if ((slave->freezeState == SLAVE_FREEZE) && (slave->joinBuddy == master)) {
        }
        else {
            // TODO сделать нормальные сравнения с объединёнными переменными
            if ((slave->freezeState == REQUEST_SLAVE) && (slave->joinBuddy == master)) {
                if (masterKey < slaveKey) {
                    expState = REQUEST_SLAVE;
                    expJoinBuddy = slave;
                    // TODO сделать нормальный кас в вайл вставить
                    if ((master->freezeState == expState) && (master->joinBuddy == expJoinBuddy)) {
                        casResult = master->freezeState.compare_exchange_weak(expState, SLAVE_FREEZE);
                        casResult = casResult && master->joinBuddy.compare_exchange_weak(expJoinBuddy, slave);
                    }

                    return casResult;
                }
                else {
                    expState = REQUEST_SLAVE;
                    expJoinBuddy = master;
                    // TODO сделать нормальный кас в вайл вставить
                    if ((master->freezeState == expState) && (master->joinBuddy == expJoinBuddy)) {
                        casResult = master->freezeState.compare_exchange_weak(expState, SLAVE_FREEZE);
                        casResult = casResult && master->joinBuddy.compare_exchange_weak(expJoinBuddy, master);
                    }

                    return casResult;
                }
            }
            bool result;
            Freeze(slave, 0, nullptr, master, TT_ENSLAVE , &result);
            return false;
        }
    }
    MarkChunkFrozen(slave->chunk);
    StabilizeChunk(slave->chunk);
    return true;
}

/*
 * Устанавливает вместо двух узлов между которыми установлено отношение master-slave
 * два новых узла.
 * Первый узел содержится в поле neww у master, а второй в поле nextNew у neww
 * slave находится в поле joinBuddy у master
 */
void LockFreeTree::InsertBorrowNodes(LockFreeElement *master, short int sepKey) {
    LockFreeElement* n1 = master->neww;
    LockFreeElement* n2 = master->neww.load(std::memory_order_relaxed)->nextNew;
    // TODO сделать нормальное присвоение с объединёнными переменнами
    LockFreeElement *slave = master->joinBuddy;

    short int maxMasterKey = getMaxKey(master);
    short int maxSlaveKey = getMaxKey(slave);

    short int highKey;
    short int lowKey;
    LockFreeElement *oldHigh;
    LockFreeElement *oldLow;
    if (maxSlaveKey < maxMasterKey) {
        highKey = maxMasterKey;
        oldHigh = master;
        lowKey = maxSlaveKey;
        oldLow = slave;
    }
    else {
        highKey = maxSlaveKey;
        oldHigh = slave;
        lowKey = maxMasterKey;
        oldLow = master;
    }

    LockFreeElement *sepKeyNode;
    if (lowKey < sepKey) {
        sepKeyNode = oldHigh;
    }
    else {
        sepKeyNode = oldLow;
    }

    LockFreeElement *insertParent;
    Entry *ent;
    if ((insertParent = FindParent(sepKey, sepKeyNode, &ent, nullptr)) != nullptr) {
        InsertToChunk(insertParent, sepKey, n1);
    }
    LockFreeElement *highParent;
    Entry *highEnt;
    if ((highParent = FindParent(highKey, oldHigh, &highEnt, nullptr)) != nullptr) {
        ReplaceInChunk(highParent, highEnt->key,
                       combine(highEnt->key, oldHigh), combine(highEnt->key, n2));
    }
    LockFreeElement *lowParent;
    Entry *lowEnt;
    if ((lowParent = FindParent(lowKey, oldLow, &lowEnt, nullptr)) != nullptr) {
        DeleteInChunk(lowParent,lowEnt->key);
    }

    short int expState = INFANT;
    LockFreeElement *expJoinBuddy = nullptr;
    // TODO сделать нормальный кас в вайл вставить
    if ((master->freezeState == expState) && (master->joinBuddy == expJoinBuddy)) {
        master->freezeState.compare_exchange_weak(expState, NORMAL);
        master->joinBuddy.compare_exchange_weak(expJoinBuddy, nullptr);
    }
    // TODO сделать нормальный кас в вайл вставить
    if ((master->freezeState == expState) && (master->joinBuddy == expJoinBuddy)) {
        master->freezeState.compare_exchange_weak(expState, NORMAL);
        master->joinBuddy.compare_exchange_weak(expJoinBuddy, nullptr);
    }
    return;
}

/*
 * Устанавливает вместо двух узлов между которыми установлено отношение master-slave
 * один новый узл.
 * Узел содержится в поле neww у master
 * slave находится в поле joinBuddy у master
 */
void LockFreeTree::InsertMergeNode(LockFreeElement *master) {
    LockFreeElement *neww = master->neww;
    // TODO сделать нормальное присвоение с объединёнными переменнами
    LockFreeElement *slave = master->joinBuddy;

    short int maxMasterKey = getMaxKey(master);
    short int maxSlaveKey = getMaxKey(slave);

    short int highKey;
    short int lowKey;
    LockFreeElement *highNode;
    LockFreeElement *lowNode;
    if (maxSlaveKey < maxMasterKey) {
        highKey = maxMasterKey;
        highNode = master;
        lowKey = maxSlaveKey;
        lowNode = slave;
    }
    else {
        highKey = maxSlaveKey;
        highNode = slave;
        lowKey = maxMasterKey;
        lowNode = master;
    }

    LockFreeElement *parent;
    Entry *highEnt;
    if ((parent = FindParent(highKey, highNode, &highEnt, nullptr )) != nullptr) {
        short int highEntKey = highEnt->key;
        ReplaceInChunk(parent, highEntKey,
                combine(highEntKey, highNode), combine(highEntKey, neww));
    }
    Entry *lowEnt;
    if ((parent = FindParent(lowKey, lowNode, &lowEnt, nullptr)) != nullptr) {
        if (parent == root) {
            MergeRoot(parent, neww, lowNode, lowEnt->key);
        }
        else {
            DeleteInChunk(parent, lowEnt->key);
        }
    }

    short int expState = INFANT;
    LockFreeElement *expJoinBuddy = nullptr;
    // TODO сделать нормальный кас в вайл вставить
    if ((master->freezeState == expState) && (master->joinBuddy == expJoinBuddy)) {
        master->freezeState.compare_exchange_weak(expState, NORMAL);
        master->joinBuddy.compare_exchange_weak(expJoinBuddy, nullptr);
    }
}

/*
 * Попытка обновление дерева
 * freezeState - состояние узла
 * node - узел, который хочет обновить дерево
 * sepKey - возможный ключ сепаратор, если сплит или борроу
 */
void LockFreeTree::CallForUpdate(short int freezeState, LockFreeElement *node, short int sepKey) {
    LockFreeElement *n1 = node->neww;
    LockFreeElement *n2 = node->neww.load(std::memory_order_relaxed)->nextNew;
    switch (freezeState) {
        case COPY: {
            LockFreeElement *parent;
            Entry *nodeEnt;
            if (node == root) {
                root.compare_exchange_weak(node, n1);
            } else if (
                    (parent = FindParent(node->chunk->head->next.load(std::memory_order_relaxed)->key, node, &nodeEnt,
                                         nullptr)) != nullptr) {
                ReplaceInChunk(parent, node->chunk->head->next.load(std::memory_order_relaxed)->key,
                               combine(nodeEnt->key, node), combine(nodeEnt->key, n1));
            }

            short int expState = INFANT;
            LockFreeElement *expJoinBuddy = nullptr;
            // TODO сделать нормальный кас в вайл вставить
            if ((n1->freezeState == expState) && (n1->joinBuddy == expJoinBuddy)) {
                n1->freezeState.compare_exchange_weak(expState, NORMAL);
                n1->joinBuddy.compare_exchange_weak(expJoinBuddy, nullptr);
            }

            return;
        }
        case SPLIT: {
            if (node == root) {
                SplitRoot(node, sepKey, n1, n2);
            } else {
                InsertSplitNodes(node, sepKey);
            }
            return;
        }
        case JOIN: {
            if (n2 == nullptr) {
                InsertMergeNode(node);
            } else {
                InsertBorrowNodes(node, sepKey);
            }

            return;
        }
        default:
            break;
    }
}

/*
 * Помощь узлу, которы находится в состоянии INFANT
 * Стать нормальным
 */
void LockFreeTree::helpInfant(LockFreeElement *node) {
    LockFreeElement *creator = node->creator;
    // TODO сделать нормальное присвоение с объединёнными переменнами
    short int creatorFrSt = creator->freezeState;
    LockFreeElement *n1 = creator->neww;
    LockFreeElement *n2 = creator->neww.load(std::memory_order_relaxed)->nextNew;

    short int sepKey = getMaxKey(n1);

    // TODO сделать нормальные сравнения с объединёнными переменными
    if (n1->freezeState != INFANT) {
        if (n2) {
            short int expState = INFANT;
            LockFreeElement *expJoinBuddy = nullptr;
            // TODO сделать нормальный кас в вайл вставить
            if ((n2->freezeState == expState) && (n2->joinBuddy == expJoinBuddy)) {
                n2->freezeState.compare_exchange_weak(expState, NORMAL);
                n2->joinBuddy.compare_exchange_weak(expJoinBuddy, nullptr);
            }
        }
        return;
    }
    if ((creator == root) && (creatorFrSt == SPLIT)) {

        short int expState = INFANT;
        LockFreeElement *expJoinBuddy = nullptr;
        // TODO сделать нормальный кас в вайл вставить
        if ((n1->freezeState == expState) && (n1->joinBuddy == expJoinBuddy)) {
            n1->freezeState.compare_exchange_weak(expState, NORMAL);
            n1->joinBuddy.compare_exchange_weak(expJoinBuddy, nullptr);
        }
        // TODO сделать нормальный кас в вайл вставить
        if ((n2->freezeState == expState) && (n2->joinBuddy == expJoinBuddy)) {
            n2->freezeState.compare_exchange_weak(expState, NORMAL);
            n2->joinBuddy.compare_exchange_weak(expJoinBuddy, nullptr);
        }

        return;
    }

    switch (creatorFrSt) {
        case COPY: {
            short int expState = INFANT;
            LockFreeElement *expJoinBuddy = nullptr;
            // TODO сделать нормальный кас в вайл вставить
            if ((node->freezeState == expState) && (node->joinBuddy == expJoinBuddy)) {
                node->freezeState.compare_exchange_weak(expState, NORMAL);
                node->joinBuddy.compare_exchange_weak(expJoinBuddy, nullptr);
            }

            return;
        }
        case SPLIT: {
            InsertSplitNodes(creator, sepKey);
            return;
        }
        case JOIN: {
            if (n2 == nullptr) {
                InsertMergeNode(creator);
            } else {
                InsertBorrowNodes(creator, sepKey);
            }

            return;
        }
        default:
            break;
    }
}

/*
 * Разделение корня на два целевых узла
 * У дерева получается новый корень
 */
void LockFreeTree::SplitRoot(LockFreeElement *oldRoot, short int sepKey, LockFreeElement *n1,
               LockFreeElement *n2) {
    LockFreeElement *newRoot = Allocate();
    // TODO сделать нормальное присвоение с объединёнными переменнами
    newRoot->freezeState = NORMAL;
    newRoot->joinBuddy = nullptr;
    newRoot->height = oldRoot->height + 1;
    addRootSons(newRoot, sepKey, n1, INF, n2);

    root.compare_exchange_weak(oldRoot, newRoot);

    short int expState = INFANT;
    LockFreeElement *expJoinBuddy = nullptr;
    // TODO сделать нормальный кас в вайл вставить
    if ((n1->freezeState == expState) && (n1->joinBuddy == expJoinBuddy)) {
        n1->freezeState.compare_exchange_weak(expState, NORMAL);
        n1->joinBuddy.compare_exchange_weak(expJoinBuddy, nullptr);
    }
    // TODO сделать нормальный кас в вайл вставить
    if ((n2->freezeState == expState) && (n2->joinBuddy == expJoinBuddy)) {
        n2->freezeState.compare_exchange_weak(expState, NORMAL);
        n2->joinBuddy.compare_exchange_weak(expJoinBuddy, nullptr);
    }

    return;
}

/*
 * Слияние двух узлов, так что остаётся только один корень
 */
void LockFreeTree::MergeRoot(LockFreeElement *oldRoot, LockFreeElement *posibleNewRoot,
               LockFreeElement * c1, short int c1Key) {
    Entry firstEnt;
    Entry secondEnt;
    int rootEntNum = getEntNum(oldRoot->chunk, &firstEnt, &secondEnt);
    if (rootEntNum > 2) {
        DeleteInChunk(oldRoot, c1Key);
        return;
    }
    if ((firstEnt.data == c1) && (secondEnt.data == posibleNewRoot)) {
        root.compare_exchange_weak(oldRoot, posibleNewRoot);
    }
    return;
}

/*
 * Замена в целевом чанке записи с данным ключом на neww, но если
 * нынешнее значение равно exp
 */
bool LockFreeTree::ReplaceInChunk(LockFreeElement *node, short int key, LockFreeElement *exp,
                                  LockFreeElement *neww) {
    FindResult findResult;
    Find(node->chunk, key, &findResult);
    if (findResult.cur == nullptr) {
        return false;
    }
    if (!findResult.cur->data.compare_exchange_weak(exp, neww)) {
        // TODO сделать нормальные сравнения с объединёнными переменными
        if (isFrozen(findResult.cur)) {
            bool result;

            Chunk *tmpChunk = Freeze(node, key, exp, neww, TT_REPLACE , &result);
            if (tmpChunk == nullptr) {
                return result;
            }
        }
        return false;
    }
    return true;
}

/*
 * На вход даётся чанк, у которого все записи заморожены и не должны изменяться
 * Метод удаляет все записи, помеченнуы как удалённые (через метод Find)
 * И вставляет все записи, которые ещё не вставлены в список но выделены
 */
void LockFreeTree::StabilizeChunk(Chunk *chunk) {
    short int maxKey = INF;
    FindResult findResult;
    // Удаление всех помеченных на удаление записей через файнд
    Find(chunk, maxKey, &findResult);
    Entry *current = chunk->head->next;
    while (current != nullptr) {
        short int key = current->key;
        Entry *eNext = current->next;
        // Вставка всех выделенных записей, которые не в списке
        if ( (key != EMPTY) && (!isDeleted(eNext))) {
            Find(chunk, key, &findResult);
            if (findResult.cur == nullptr) {
                InsertEntry(chunk, current, key);
            }
        }
    }
    return;
}

/*
 * Замораживает весь чанк, включая записи, котоыре не в списке
 */
void LockFreeTree::MarkChunkFrozen(Chunk *chunk) {
    // TODO
}

/*
 * Выбор действия, которые надо воспроизвести с чанком, вышел ли он за пределы значений MAX и MIN
 */
short int LockFreeTree::FreezeDecision(Chunk *chunk) {
    //Entry *e = chunk->head->next;
    int cnt = 0;
    /*
     * TODO WTF???
    while (clearFrozen(e) != NULL) {
        cnt++;
        e = e->next;
    }
     */
    if (cnt <= MIN) {
        return RT_MERGE;
    }
    if (cnt >= MAX) {
        return RT_SPLIT;
    }
    return RT_COPY ;
}

/*
 * Замораживание узла
 * С поиском всех нужных узлов для мерджа например
 * Возвращает null если всё окки
 * Обозначение всех входных данных смотри в FreezeRecovery,
 * так как он является логическим продолжением
 */
Chunk* LockFreeTree::Freeze(LockFreeElement *node, short int key, LockFreeElement *expected,
              LockFreeElement *data, short int tgr, bool *res) {
    short int expState = NORMAL;
    LockFreeElement *expJoinBuddy = nullptr;
    // TODO сделать нормальный кас в вайл вставить
    if ((node->freezeState == expState) && (node->joinBuddy == expJoinBuddy)) {
        node->freezeState.compare_exchange_weak(expState, FREEZE);
        node->joinBuddy.compare_exchange_weak(expJoinBuddy, nullptr);
    }

    short int decision = RT_MERGE;
    LockFreeElement *mergePartner = nullptr;
    // TODO сделать нормальные сравнения с объединёнными переменными
    switch (node->freezeState) {
        case COPY: {
            decision = RT_COPY;
            break;
        }
        case SPLIT: {
            decision = RT_SPLIT;
            break;
        }
        case JOIN: {
            decision = RT_MERGE;
            // TODO сделать нормальное присвоение с объединёнными переменнами
            mergePartner = node->joinBuddy;
            break;
        }
        case REQUEST_SLAVE: {
            decision = RT_MERGE;
            mergePartner = FindJoinSlave(node, nullptr);
            if (mergePartner != nullptr) {
                break;
            }
        }
        case SLAVE_FREEZE: {
            decision = RT_MERGE;
            // TODO сделать нормальное присвоение с объединёнными переменнами
            mergePartner = node->joinBuddy;

            LockFreeElement *tmp = mergePartner;
            mergePartner = node;
            node = tmp;

            MarkChunkFrozen(mergePartner->chunk);
            StabilizeChunk(mergePartner->chunk);

            expState = REQUEST_SLAVE;
            expJoinBuddy = mergePartner;
            // TODO сделать нормальный кас в вайл вставить
            if ((node->freezeState == expState) && (node->joinBuddy == expJoinBuddy)) {
                node->freezeState.compare_exchange_weak(expState, JOIN);
                node->joinBuddy.compare_exchange_weak(expJoinBuddy, mergePartner);
            }
            break;
        }
        case FREEZE: {
            MarkChunkFrozen(node->chunk);
            StabilizeChunk(node->chunk);
            decision = FreezeDecision(node->chunk);

            switch (decision) {
                case RT_COPY: {
                    expState = FREEZE;
                    expJoinBuddy = nullptr;
                    // TODO сделать нормальный кас в вайл вставить
                    if ((node->freezeState == expState) && (node->joinBuddy == expJoinBuddy)) {
                        node->freezeState.compare_exchange_weak(expState, COPY);
                        node->joinBuddy.compare_exchange_weak(expJoinBuddy, nullptr);
                    }
                    break;
                }
                case RT_SPLIT: {
                    expState = FREEZE;
                    expJoinBuddy = nullptr;
                    // TODO сделать нормальный кас в вайл вставить
                    if ((node->freezeState == expState) && (node->joinBuddy == expJoinBuddy)) {
                        node->freezeState.compare_exchange_weak(expState, SPLIT);
                        node->joinBuddy.compare_exchange_weak(expJoinBuddy, nullptr);
                    }
                    break;
                }
                case RT_MERGE: {
                    mergePartner = FindJoinSlave(node, nullptr);
                    if (mergePartner == nullptr) {
                        mergePartner = node;
                        // TODO сделать нормальное присвоение с объединёнными переменнами
                        node = node->joinBuddy;

                        expState = REQUEST_SLAVE;
                        expJoinBuddy = mergePartner;
                        // TODO сделать нормальный кас в вайл вставить
                        if ((node->freezeState == expState) && (node->joinBuddy == expJoinBuddy)) {
                            node->freezeState.compare_exchange_weak(expState, JOIN);
                            node->joinBuddy.compare_exchange_weak(expJoinBuddy, mergePartner);
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }
    return FreezeRecovery(node, key, expected, data, decision, mergePartner, tgr, res);
}

/*
 * Восстановление замороженного узла oldNode с ключом в родительском узле key
 * recovType - содержит в себе значения, что надо делать с узлом
 * input и expected будут в случае, если пошли изменятся не листы
 * input - входное значение узла, который вызвал фриз, он есть в случае вставки или замены
 * expected - ожидаемое значение узла в случае замены узла
 * mergePartner - узел, который является партнёром по мерджу, если он конечно есть
 * trigger - что вызвало фриз узла (в общем интересен только ENSLAVE
 * так как в данном случае мы сразу заканчиваем порабощение узла для мёрджа узла)
 * в result записывается успешность всей вставки узла
 */
Chunk* LockFreeTree::FreezeRecovery(LockFreeElement *oldNode, short int key,
                      LockFreeElement *expected, LockFreeElement *input,
                      short int recovType, LockFreeElement *mergePartner,
                      short int trigger, bool *result) {
    LockFreeElement *retNode = nullptr;
    short int sepKey = INF;
    LockFreeElement *newNode1 = Allocate();
    newNode1->creator = oldNode;
    LockFreeElement *newNode2 = nullptr;

    switch (recovType) {
        case RT_COPY: {
            copyToOneChunkNode(oldNode, newNode1);
            break;
        }
        case RT_MERGE: {
            Entry firstEnt;
            Entry secondEnt;
            int numberEnt = getEntNum(oldNode->chunk, &firstEnt, &secondEnt) +
                            getEntNum(mergePartner->chunk, &firstEnt, &secondEnt);
            if (numberEnt >= MAX) {
                newNode2 = Allocate();
                newNode1->nextNew = newNode2;
                newNode2->creator = oldNode;
                sepKey = mergeToTwoNodes(oldNode, mergePartner, newNode1, newNode2);
            } else {
                mergeToOneNode(oldNode, mergePartner, newNode1);
            }
            break;
        }
        case RT_SPLIT: {
            newNode2 = Allocate();
            newNode1->nextNew = newNode2;
            newNode2->creator = oldNode;
            sepKey = splitIntoTwoNodes(oldNode, newNode1, newNode2);
            break;
        }
        default:
            break;
    }

    if ((newNode2 != nullptr) && (newNode2->height != 0)) {
        LockFreeElement *leftNode = getMaxEntry(newNode1)->data;
        // TODO сделать нормальное присвоение с объединёнными переменнами
        short int leftState = leftNode->freezeState;
        LockFreeElement *rightNode = newNode2->chunk->head->next.load(std::memory_order_relaxed)->data;
        // TODO сделать нормальное присвоение с объединёнными переменнами
        short int rightState = rightNode->freezeState;

        // TODO сделать нормальные сравнения с объединёнными переменными
        if ((rightState == REQUEST_SLAVE) && ((leftState == NORMAL ) || (leftState == INFANT ) || (leftState == FREEZE ) ||
                (leftState == COPY ) || (leftState == SLAVE_FREEZE/*<SLAVE FREEZE , rightNode>*/))) {
            moveEntryFromFirstToSecond(newNode1, newNode2);
            sepKey = getMaxEntry(newNode1)->key;
        }
        // TODO сделать нормальные сравнения с объединёнными переменными
        else if (rightState == JOIN/*< MERGE , leftNode>*/) {
            moveEntryFromFirstToSecond(newNode1, newNode2);
            sepKey = getMaxEntry(newNode1)->key;
        }
        else if ((rightState == INFANT) && (leftState == SLAVE_FREEZE)) {
            // TODO сделать нормальные сравнения с объединёнными переменными
            if (rightNode->creator == leftNode->joinBuddy) {
                moveEntryFromFirstToSecond(newNode1, newNode2);
                sepKey = (getMaxEntry(newNode1))->key;
            }
        }
    }

    switch (trigger) {
        case TT_DELETE:
            *result = DeleteInChunk(newNode1, key);
            if (newNode2 != nullptr) {
                *result = *result || DeleteInChunk(newNode2, key);
            }
            break;
        case TT_INSERT:
            if ((newNode2 != nullptr) && (key < sepKey)) {
                *result = InsertToChunk(newNode2, key, input);
            }
            else {
                *result = InsertToChunk(newNode1, key, input);
            }
            break;
        case TT_REPLACE:
            if ((newNode2!= nullptr) && (key < sepKey)) {
                *result = ReplaceInChunk(newNode2, key, expected, input);
            }
            else {
                *result = ReplaceInChunk(newNode1, key, expected, input);
            }
            break;
        case TT_ENSLAVE:
            if (recovType == RT_COPY) {
                // TODO сделать нормальное присвоение с объединёнными переменнами
                newNode1->freezeState = SLAVE_FREEZE;
                newNode1->joinBuddy = input;
            }
        default:
            break;
    }

    LockFreeElement *tmp = nullptr;
    if (!oldNode->neww.compare_exchange_weak(tmp, newNode1)) {
        if (key <= sepKey) {
            retNode = oldNode->neww;
        }
        else {
            retNode = oldNode->neww.load(std::memory_order_relaxed)->nextNew;
        }
    }

    // TODO сделать нормальные сравнения с объединёнными переменными
    if (newNode1->freezeState == SLAVE_FREEZE) {
        // TODO сделать нормальное присвоение с объединёнными переменнами
        LockFreeElement *m = newNode1->joinBuddy;

        short int expState = REQUEST_SLAVE;
        LockFreeElement *expJoinBuddy = oldNode;
        // TODO сделать нормальный кас в вайл вставить
        if ((m->freezeState == expState) && (m->joinBuddy == expJoinBuddy)) {
            m->freezeState.compare_exchange_weak(expState, REQUEST_SLAVE);
            m->joinBuddy.compare_exchange_weak(expJoinBuddy, newNode1);
        }
    }
    CallForUpdate(recovType, oldNode, key);

    Chunk *resultChunk = nullptr;
    if (retNode != nullptr) {
        resultChunk = retNode->chunk;
    }

    return resultChunk;
}

// те, которые не известны ==========================================================================
/*
 * Поиск узла в данном чанке с ключом равному или
 * записи, ключ которой больше заданного, но минимальный среди таких
 * Результат должен записывать в глобальную переменную *cur,
 * что я считаю бредом, мб стоит
 * Также записывает в глобальную переменные **prev и *next записи, которые идут до и после
 * cur, если бы список был упорядочен
 * Почему **prev, а не *prev:
 * Сделана структура FindResult, которая содержит результаты
 */
void LockFreeTree::Find(Chunk *chunk, short int key, FindResult *findResult) {

}

/*
 * Возврат нормального указателя * вместо **
 */
Entry* LockFreeTree::EntPtr(Entry **entry) {
    return *entry;
}

/*
 * Поиск в данном чанке записи с ключом key
 */
bool LockFreeTree::SearchInChunk(Chunk *chunk, short int key) {

    return false;
}

/*
 * Вставка в чанк записи, которая содержит данный ключ и данную ссылку на узел
 */
bool LockFreeTree::InsertToChunk(LockFreeElement *node, short int key, LockFreeElement *data) {

    return false;
}

/*
 * Удаление в данном чанке записи с данным ключом
 */
bool LockFreeTree::DeleteInChunk(LockFreeElement *node, short int key) {

    return false;
}

/*
 * Выдаёт максимальный ключ в списке
 */
short int LockFreeTree::getMaxKey(LockFreeElement *node) {

    return 0;
}

/*
 * TODO Получает две переменные с каким-то количеством битов, объединяет их в одно машинное слово с этим пока проблемы
 * Но судя по всему просто берёт ключ и ссылку на узел и делает запись, которая содержит их
 */
LockFreeElement* LockFreeTree::combine(short int key, LockFreeElement *node) {

    return nullptr;
}

/*
 * Создание нового пустого узла
 */
LockFreeElement* LockFreeTree::Allocate() {
    LockFreeElement *result = new LockFreeElement(MAX);
    nodes.push_back(result);
    return result;
}

/*
 * Добавление в рут два данных сына с данными ключами
 */
void LockFreeTree::addRootSons(LockFreeElement *root, short int sepKey1, LockFreeElement *n1,
                 short int sepKey2, LockFreeElement *n2) {

}

/*
 * Выдаёт сколько записей содержится в чанке
 * Также записывает в firstEnt и secondEnt
 * ссылку на первую и вторую запись соотвественно
 * Если рассматривать список как упорядочный
 */
int LockFreeTree::getEntNum(Chunk *chunk, Entry *firstEnt, Entry *secondEnt) {

    return 0;
}

/*
 * Проверка на то заморожена ли запись
 */
bool LockFreeTree::isFrozen(Entry *entry) {

    return false;
}

/*
 * Проверка на то удалена ли запись
 */
bool LockFreeTree::isDeleted(Entry *entry) {

    return false;
}

/*
 * Вставка записи в данный чанк, для удобства передаётся ключ, возвращает код успешности
 */
short int LockFreeTree::InsertEntry(Chunk *chunk, Entry *e, short int key) {

}

/*
 * Помечает переданную запись как зафриженую, возвращает эту же запись для удобства
 */
Entry* LockFreeTree::MarkFrozen(Entry* entry) {

}

/*
 * Снимает пометку о зафриженности у переданной записи, возвращает эту же запись для удобства
 */
Entry* LockFreeTree::clearFrozen(Entry* entry) {

}

/*
 * Копирует содержимое чанка oldNode в newNode
 */
void LockFreeTree::copyToOneChunkNode(LockFreeElement *oldNode, LockFreeElement* newNode) {

}

/*
 * Записывает в чанки двух новых узлов практически идентичное значение
 * чанков oldNode и mergePartner
 * В первый узел записывается ключи из узла с меньшими ключами
 * Во второй с большими
 * В тот узел, который повторяет чанк oldNode,
 * должен быть записан самый первый или самый последний ключ
 * из узла mergePartner, если в нём содержатся большие ключи или меньшие соотвественно
 * Также возвращается ключ-сепаратор, это самый большой ключ из узла,
 * в котором записаны все меньшие ключи
 */
short int LockFreeTree::mergeToTwoNodes(LockFreeElement *oldNode, LockFreeElement *mergePartner,
                          LockFreeElement *newNode1, LockFreeElement *newNode2) {

    return 0;
}

/*
 * В чанк нового узла записываются все значения из oldNode и mergePartner соотвественно
 */
void LockFreeTree::mergeToOneNode(LockFreeElement *oldNode, LockFreeElement *mergePartner,
                    LockFreeElement *newNode) {

}

/*
 * Разделение одного узла на два
 * Находится ключ-сепаратор - медиана ключей в oldNode
 * В первый узел записываются все записи с ключами меньше или равными, чем ключ-сепаратор
 * Во второй все записи, которые больше ключа-сепаратора
 */
short int LockFreeTree::splitIntoTwoNodes(LockFreeElement *oldNode, LockFreeElement *newNode1,
                            LockFreeElement *newNode2) {

    return 0;
}

/*
 * Перемещение первой записи (если бы список был упорядочен) из first
 * в second
 */
void LockFreeTree::moveEntryFromFirstToSecond(LockFreeElement *first, LockFreeElement *second) {

}

/*
 * Выдаёт запись с максимальным ключом
 */
Entry* LockFreeTree::getMaxEntry(LockFreeElement *node) {

    return nullptr;
}

/*
 * Выделяет новую запись в данном чанке с данным ключом и данными
 */
Entry* LockFreeTree::AllocateEntry(Chunk *chunk, short int key, LockFreeElement *data) {

    return nullptr;
}

/*
 * Удаляет помеченную как удалённую запись из списка
 */
void LockFreeTree::RetireEntry(Entry *entry) {

}

/*
 * Очищает запись (присваевает ей специальный ключ)
 */
bool LockFreeTree::ClearEntry(Chunk *chunk, Entry *entry) {

    return false;
}

/*
 * Увеличивает каунтер элементов в узле
 */
void LockFreeTree::IncCount(LockFreeElement *node) {

}

/*
 * Уменьшает каунтер элементов в узле, если не превышен порог MIN
 */
bool LockFreeTree::DecCount(LockFreeElement *node) {

    return false;
}

/*
 * Помечает переданную запись как зафриженую, возвращает эту же запись для удобства
 */
Entry* LockFreeTree::MarkDeleted(Entry* entry) {

    return nullptr;
}

/*
 * Снимает пометку о зафриженности у переданной записи, возвращает эту же запись для удобства
 */
Entry* LockFreeTree::clearDeleted(Entry* entry) {

    return nullptr;
}