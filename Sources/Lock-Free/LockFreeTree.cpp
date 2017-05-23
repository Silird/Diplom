#include <iostream>
#include "../../Headers/Lock-Free/LockFreeTree.h"

LockFreeTree::LockFreeTree(int range) {
    MAX = range;
    MIN = range/2 - 3;
    root = Allocate();
    root.load(std::memory_order_relaxed)->setFreezeState(NORMAL);

    LockFreeElement *tmp = root;
    unsigned short int freeze = tmp->getFreezeState();

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
    if (node->getFreezeState() == INFANT) {
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
    if (node->getFreezeState() == INFANT) {
        helpInfant(node);
    }
    return DeleteInChunk(node, key);
}

void LockFreeTree::print() {
    LockFreeElement *tmpRoot = root;
    if (tmpRoot->chunk->head->getNext() == nullptr) {
        std::cout << "Tree is empty" << std::endl;
    }
    else {
        std::cout << "Tree:" << std::endl;
        print(tmpRoot, tmpRoot->height);
    }
}

void LockFreeTree::printValues() {
    LockFreeElement *tmpRoot = root;
    if (tmpRoot->chunk->head->getNext() == nullptr) {
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
        Entry *current = element->chunk->head->getNext();
        while (current != nullptr) {
            for (int i = 0; i < maxHeight - element->height; i++) {
                if (i == (element->height - 1)) {
                    std::cout << "|--";
                }
                else {
                    std::cout << "|  ";
                }
            }
            std::cout << current->getKey();
            if (element->height == 0) {
                std::cout << " (value)";
            }
            else {
                std::cout << " (link)";
            }

            std::cout << std::endl;

            current = current->getNext();
        }

        std::cout << "|" << std::endl;

        current = element->chunk->head->getNext();
        while (current != nullptr) {
            print(current->getData(), maxHeight);

            current = current->getNext();
        }
    }
}

void LockFreeTree::printValues(LockFreeElement *element) {
    if (element != nullptr) {
        if (element->height != 0) {
            Entry *current = element->chunk->head->getNext();
            while (current != nullptr) {
                printValues(current->getData());

                current = current->getNext();
            }
        }
        else {
            Entry *current = element->chunk->head->getNext();
            while (current != nullptr) {
                std::cout << current->getKey() << " ";

                current = current->getNext();
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
        node = findResult.cur->getData();
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
        if (childNode == findResult.cur->getData()) {
            *prntEnt = findResult.cur;
            if (slaveEnt != nullptr) {
                if (findResult.prev == &(node->chunk->head)) {
                    *slaveEnt = findResult.next;
                }
                else {
                    *slaveEnt = EntPtr(findResult.prev);
                }
            }
            if (node->getFreezeState() == INFANT) {
                helpInfant(node);
            }
            return node;
        }
        node = findResult.cur->getData();
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
        ReplaceInChunk(parent, nodeEnt->getKey(),
                combine(nodeEnt->getKey(), node), combine(nodeEnt->getKey(), n2));
    }

    NodeState nodeStateExp(INFANT);
    n1->state.compare_exchange_weak(nodeStateExp, NodeState(NORMAL));
    nodeStateExp.freezeState = INFANT;
    nodeStateExp.joinBuddy = nullptr;
    n2->state.compare_exchange_weak(nodeStateExp, NodeState(NORMAL));

    return;
}

/*
 * Находит раба для мёрджа с master
 * Устанавливет состояния у обоих узлов
 */
LockFreeElement* LockFreeTree::FindJoinSlave(LockFreeElement *master, LockFreeElement *oldSlave) {
    short int anyKey = master->chunk->head->getNext()->getKey();
    LockFreeElement *parent;
    Entry *masterEnt;
    Entry *slaveEnt;
    if ((parent = FindParent(anyKey, master, &masterEnt, &slaveEnt)) == nullptr) {
        return master->getJoinBuddy();
    }
    LockFreeElement *slave = slaveEnt->getData();

    NodeState expState;
    if (oldSlave == nullptr) {
        expState = NodeState(FREEZE);
    }
    else  {
        expState = NodeState(REQUEST_SLAVE, oldSlave);
    }

    if (!master->state.compare_exchange_weak(expState, NodeState(REQUEST_SLAVE, slave))) {
        if (master->getFreezeState() == JOIN) {
            return master->getJoinBuddy();
        }
    }
    slave = master->getJoinBuddy();
    if ((parent->getFreezeState() != NORMAL) && (oldSlave == nullptr)) {
        bool result;
        Freeze(parent, 0, combine(EMPTY, nullptr), combine(masterEnt->getKey(), master), TT_NONE, &result);
        FindJoinSlave(master, slave);
    }
    if (!SetSlave(master, slave, masterEnt->getKey(), slave->chunk->head->getNext()->getKey())) {
        FindJoinSlave(master, slave);
    }

    expState.freezeState = REQUEST_SLAVE;
    expState.joinBuddy = slave;
    master->state.compare_exchange_weak(expState, NodeState(JOIN, slave));

    if (master->getFreezeState() == JOIN) {
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

    NodeState expState(NORMAL);
    if (!master->state.compare_exchange_weak(expState, NodeState(SLAVE_FREEZE, master))) {
        if (slave->getFreezeState() == INFANT) {
            helpInfant(slave);
            return false;
        }
        else if ((slave->getFreezeState() == SLAVE_FREEZE) && (slave->getJoinBuddy() == master)) {
        }
        else {
            if ((slave->getFreezeState() == REQUEST_SLAVE) && (slave->getJoinBuddy() == master)) {
                if (masterKey < slaveKey) {
                    expState.freezeState = REQUEST_SLAVE;
                    expState.joinBuddy = slave;
                    // TODO ??? expState = REQUEST_SLAVE, slave;

                    return master->state.compare_exchange_weak(expState, NodeState(SLAVE_FREEZE, slave));
                }
                else {
                    expState.freezeState = REQUEST_SLAVE;
                    expState.joinBuddy = master;

                    return slave->state.compare_exchange_weak(expState, NodeState(SLAVE_FREEZE, master));
                }
            }
            bool result;
            Freeze(slave, 0, combine(EMPTY, nullptr), combine(masterKey, master), TT_ENSLAVE , &result);
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
    LockFreeElement *slave = master->getJoinBuddy();

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
        ReplaceInChunk(highParent, highEnt->getKey(),
                       combine(highEnt->getKey(), oldHigh), combine(highEnt->getKey(), n2));
    }
    LockFreeElement *lowParent;
    Entry *lowEnt;
    if ((lowParent = FindParent(lowKey, oldLow, &lowEnt, nullptr)) != nullptr) {
        DeleteInChunk(lowParent, lowEnt->getKey());
    }

    NodeState expState(INFANT);
    n1->state.compare_exchange_weak(expState, NodeState(NORMAL));
    expState.freezeState = INFANT;
    expState.joinBuddy = nullptr;
    n2->state.compare_exchange_weak(expState, NodeState(NORMAL));

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
    LockFreeElement *slave = master->getJoinBuddy();

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
        short int highEntKey = highEnt->getKey();
        ReplaceInChunk(parent, highEntKey,
                combine(highEntKey, highNode), combine(highEntKey, neww));
    }
    Entry *lowEnt;
    if ((parent = FindParent(lowKey, lowNode, &lowEnt, nullptr)) != nullptr) {
        if (parent == root) {
            MergeRoot(parent, neww, lowNode, lowEnt->getKey());
        }
        else {
            DeleteInChunk(parent, lowEnt->getKey());
        }
    }

    NodeState expState(INFANT);
    neww->state.compare_exchange_weak(expState, NodeState(NORMAL));
}

/*
 * Попытка обновление дерева
 * freezeState - состояние узла
 * node - узел, который хочет обновить дерево
 * sepKey - возможный ключ сепаратор, если сплит или борроу
 */
void LockFreeTree::CallForUpdate(unsigned short int freezeState, LockFreeElement *node, short int sepKey) {
    LockFreeElement *n1 = node->neww;
    LockFreeElement *n2 = node->neww.load(std::memory_order_relaxed)->nextNew;
    switch (freezeState) {
        case COPY: {
            LockFreeElement *parent;
            Entry *nodeEnt;
            if (node == root) {
                root.compare_exchange_weak(node, n1);
            }
            else if ((parent = FindParent(node->chunk->head->getNext()->getKey(),
                                          node, &nodeEnt, nullptr)) != nullptr) {
                ReplaceInChunk(parent, node->chunk->head->getNext()->getKey(),
                               combine(nodeEnt->getKey(), node), combine(nodeEnt->getKey(), n1));
            }

            NodeState expState(INFANT);
            n1->state.compare_exchange_weak(expState, NodeState(NORMAL));

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
    short int creatorFrSt = creator->getFreezeState();
    LockFreeElement *n1 = creator->neww;
    LockFreeElement *n2 = creator->neww.load(std::memory_order_relaxed)->nextNew;

    short int sepKey = getMaxKey(n1);

    if (n1->getFreezeState() != INFANT) {
        if (n2) {
            NodeState expState(INFANT);
            n2->state.compare_exchange_weak(expState, NodeState(NORMAL));
        }
        return;
    }
    if ((creator == root) && (creatorFrSt == SPLIT)) {

        NodeState expState(INFANT);
        n1->state.compare_exchange_weak(expState, NodeState(NORMAL));
        expState.freezeState = INFANT;
        expState.joinBuddy = nullptr;
        n2->state.compare_exchange_weak(expState, NodeState(NORMAL));

        return;
    }

    switch (creatorFrSt) {
        case COPY: {
            NodeState expState(INFANT);
            node->state.compare_exchange_weak(expState, NodeState(NORMAL));

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
    newRoot->state = NodeState(NORMAL);
    newRoot->height = oldRoot->height + 1;
    addRootSons(newRoot, sepKey, n1, INF, n2);

    root.compare_exchange_weak(oldRoot, newRoot);

    NodeState expState(INFANT);
    n1->state.compare_exchange_weak(expState, NodeState(NORMAL));
    expState.freezeState = INFANT;
    expState.joinBuddy = nullptr;
    n2->state.compare_exchange_weak(expState, NodeState(NORMAL));

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
    if ((firstEnt.getData() == c1) && (secondEnt.getData() == posibleNewRoot)) {
        root.compare_exchange_weak(oldRoot, posibleNewRoot);
    }
    return;
}

/*
 * Замена в целевом чанке записи с данным ключом на neww, но если
 * нынешнее значение равно exp
 */
bool LockFreeTree::ReplaceInChunk(LockFreeElement *node, short int key, EntryDataKey exp,
                                  EntryDataKey neww) {
    FindResult findResult;
    Find(node->chunk, key, &findResult);
    if (findResult.cur == nullptr) {
        return false;
    }
    if (!findResult.cur->dataKey.compare_exchange_weak(exp, neww)) {
        if (isFrozen(findResult.cur->dataKey)) {
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
    for (int i = 0; i < chunk->entries.size(); i++) {
        Entry *current = chunk->entries[i];
        short int key = current->getKey();
        Entry *eNext = current->getNext();
        // Вставка всех выделенных записей, которые не в списке
        if ((key != EMPTY) && (!isDeleted(eNext))) {
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
    for (int i = 0; i < chunk->entries.size(); i++) {
        Entry *current = chunk->entries[i];

        EntryNext savedNext = current->next;
        while (!isFrozen(savedNext)) {
            current->next.compare_exchange_weak(savedNext, MarkFrozen(savedNext));
            savedNext = current->next;
        }
        EntryDataKey savedDataKey = current->dataKey;
        while (!isFrozen(savedDataKey)) {
            current->dataKey.compare_exchange_weak(savedDataKey, MarkFrozen(savedDataKey));
            savedDataKey = current->dataKey;
        }
    }
    return;
}

/*
 * Выбор действия, которые надо воспроизвести с чанком, вышел ли он за пределы значений MAX и MIN
 */
unsigned short int LockFreeTree::FreezeDecision(Chunk *chunk) {
    Entry *e = chunk->head->getNext();
    int cnt = 0;
    while (clearFrozen(e) != nullptr) {
        cnt++;
        e = e->getNext();
    }
    if (cnt <= MIN) {
        return RT_MERGE;
    }
    if (cnt >= MAX) {
        return RT_SPLIT;
    }
    return RT_COPY;
}

/*
 * Замораживание узла
 * С поиском всех нужных узлов для мерджа например
 * Возвращает null если всё окки
 * Обозначение всех входных данных смотри в FreezeRecovery,
 * так как он является логическим продолжением
 */
Chunk* LockFreeTree::Freeze(LockFreeElement *node, short int key, EntryDataKey expected,
                            EntryDataKey data, unsigned short int tgr, bool *res) {
    NodeState expState(NORMAL);
    node->state.compare_exchange_weak(expState, NodeState(FREEZE));

    unsigned short int decision = RT_MERGE;
    LockFreeElement *mergePartner = nullptr;
    switch (node->getFreezeState()) {
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
            mergePartner = node->getJoinBuddy();
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
            mergePartner = node->getJoinBuddy();

            LockFreeElement *tmp = mergePartner;
            mergePartner = node;
            node = tmp;

            MarkChunkFrozen(mergePartner->chunk);
            StabilizeChunk(mergePartner->chunk);

            expState.freezeState = REQUEST_SLAVE;
            expState.joinBuddy = mergePartner;
            node->state.compare_exchange_weak(expState, NodeState(JOIN, mergePartner));
            break;
        }
        case FREEZE: {
            MarkChunkFrozen(node->chunk);
            StabilizeChunk(node->chunk);
            decision = FreezeDecision(node->chunk);

            switch (decision) {
                case RT_COPY: {
                    expState.freezeState = FREEZE;
                    expState.joinBuddy = nullptr;
                    node->state.compare_exchange_weak(expState, NodeState(COPY));
                    break;
                }
                case RT_SPLIT: {
                    expState.freezeState = FREEZE;
                    expState.joinBuddy = nullptr;
                    node->state.compare_exchange_weak(expState, NodeState(SPLIT));
                    break;
                }
                case RT_MERGE: {
                    mergePartner = FindJoinSlave(node, nullptr);
                    if (mergePartner == nullptr) {
                        mergePartner = node;
                        node = node->getJoinBuddy();

                        expState.freezeState = REQUEST_SLAVE;
                        expState.joinBuddy = mergePartner;
                        node->state.compare_exchange_weak(expState, NodeState(JOIN, mergePartner));
                    }
                    break;
                }
                default:
                    break;
            }
        }
        default:
            break;
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
                                    EntryDataKey expected, EntryDataKey input,
                                    unsigned short int recovType, LockFreeElement *mergePartner,
                                    unsigned short int trigger, bool *result) {
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
        LockFreeElement *leftNode = getMaxEntry(newNode1)->getData();
        unsigned short int leftState = leftNode->getFreezeState();
        LockFreeElement *leftJoinBuddy = leftNode->getJoinBuddy();
        LockFreeElement *rightNode = newNode2->chunk->head->getNext()->getData();
        unsigned short int rightState = rightNode->getFreezeState();
        LockFreeElement *rightJoinBuddy = rightNode->getJoinBuddy();

        if ((rightState == REQUEST_SLAVE) && ((leftState == NORMAL) || (leftState == INFANT) || (leftState == FREEZE) ||
                (leftState == COPY) || ((leftState == SLAVE_FREEZE) && (leftJoinBuddy == rightNode)))) {
            moveEntryFromFirstToSecond(newNode1, newNode2);
            sepKey = getMaxEntry(newNode1)->getKey();
        }
        else if ((rightState == JOIN) && (rightJoinBuddy == leftNode)) {
            moveEntryFromFirstToSecond(newNode1, newNode2);
            sepKey = getMaxEntry(newNode1)->getKey();
        }
        else if ((rightState == INFANT) && (leftState == SLAVE_FREEZE)) {
            if (rightNode->creator == leftJoinBuddy) {
                moveEntryFromFirstToSecond(newNode1, newNode2);
                sepKey = (getMaxEntry(newNode1))->getKey();
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
                *result = InsertToChunk(newNode2, key, input.data);
            }
            else {
                *result = InsertToChunk(newNode1, key, input.data);
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
                newNode1->state = NodeState(SLAVE_FREEZE, input.data);
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

    if (newNode1->getFreezeState() == SLAVE_FREEZE) {
        LockFreeElement *m = newNode1->getJoinBuddy();

        NodeState expState(REQUEST_SLAVE, oldNode);
        m->state.compare_exchange_weak(expState, NodeState(REQUEST_SLAVE, newNode1));
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
 * Но судя по всему просто берёт ключ и ссылку на узел и делает запись, которая содержит их
 */
EntryDataKey LockFreeTree::combine(short int key, LockFreeElement *node) {

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
bool LockFreeTree::isFrozen(EntryDataKey dataKey) {

    return false;
}

/*
 * Проверка на то заморожена ли запись
 */
bool LockFreeTree::isFrozen(EntryNext dataKey) {

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
unsigned short int LockFreeTree::InsertEntry(Chunk *chunk, Entry *e, short int key) {

    return 0;
}

/*
 * Помечает переданную запись как зафриженую, возвращает эту же запись для удобства
 */
EntryDataKey LockFreeTree::MarkFrozen(EntryDataKey dataKey) {

}

/*
 * Помечает переданную запись как зафриженую, возвращает эту же запись для удобства
 */
EntryNext LockFreeTree::MarkFrozen(EntryNext next) {

}

/*
 * Снимает пометку о зафриженности у переданной записи, возвращает эту же запись для удобства
 */
Entry* LockFreeTree::clearFrozen(Entry* entry) {

    return nullptr;
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
EntryNext LockFreeTree::MarkDeleted(EntryNext next) {

}

/*
 * Снимает пометку о зафриженности у переданной записи, возвращает эту же запись для удобства
 */
EntryNext LockFreeTree::clearDeleted(EntryNext entry) {

}