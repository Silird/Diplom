#include <iostream>
#include "../../Headers/Lock-Free/LockFreeTree.h"

LockFreeTree::LockFreeTree(int range) {
    MAX = range;
    MIN = range/2 - 3;
    root = Allocate();
    root->freezeState = NORMAL;
    std::cout << "LockFreeTree created!" << std::endl;
}

LockFreeTree::~LockFreeTree() {
    for (int i = 0; i < nodes.size(); i++) {
        delete nodes[i];
    }
    std::cout << "LockFreeTree deleted!" << std::endl;
}

bool LockFreeTree::add(short int key) {

    return false;
}

bool LockFreeTree::search(short int key) {

    return false;
}

bool LockFreeTree::remove(short int key) {

    return false;
}

void LockFreeTree::print() {

}

void LockFreeTree::printValues() {

}


// те, которые известны

/*
 * Находит лист, у которого ПОТЕНЦИАЛЬНО может находится данный ключ
 */
LockFreeElement* LockFreeTree::FindLeaf(short int key) {

    return nullptr;
}

/*
 * Находит родительский узел, в котором содержится пара key и childNode
 * Записвает в prntEnt запись целевого узла
 * Если slaveEnt не нул, то записывает в него потенциального раба для мерджа
 */
LockFreeElement* LockFreeTree::FindParent(short int key, LockFreeElement *childNode,
                            Entry **prntEnt, Entry **slaveEnt) {

    return nullptr;
}

/*
 * Вставляет в дерево вместо узла node, два разделённых узла
 * Первый узел содержится в поле neww у node, а второй в поле nextNew у neww
 * Первый содержит меньшую часть ключей, второй большую
 * sepKey - ключ для второго узла.
 */
void LockFreeTree::InsertSplitNodes(LockFreeElement *node, short int sepKey) {

}

/*
 * Находит раба для мёрджа с master
 * Устанавливет состояния у обоих узлов
 */
LockFreeElement* LockFreeTree::FindJoinSlave(LockFreeElement *master) {

    return nullptr;
}

/*
 * Пытается установить отношение master-slave
 * Может произойти смена ролей
 */
bool LockFreeTree::SetSlave(LockFreeElement *master, LockFreeElement *slave, short int masterKey,
              short int slaveKey) {

    return false;
}

/*
 * Устанавливает вместо двух узлов между которыми установлено отношение master-slave
 * два новых узла.
 * Первый узел содержится в поле neww у master, а второй в поле nextNew у neww
 * slave находится в поле joinBuddy у master
 */
void LockFreeTree::InsertBorrowNodes(LockFreeElement *master, short int sepKey) {

}

/*
 * Устанавливает вместо двух узлов между которыми установлено отношение master-slave
 * один новый узл.
 * Узел содержится в поле neww у master
 * slave находится в поле joinBuddy у master
 */
void LockFreeTree::InsertMergeNode(LockFreeElement *master) {

}

/*
 * Попытка обновление дерева
 * freezeState - состояние узла
 * node - узел, который хочет обновить дерево
 * sepKey - возможный ключ сепаратор, если сплит или борроу
 */
void LockFreeTree::CallForUpdate(short int freezeState, LockFreeElement *node, short int sepKey) {

}

/*
 * Помощь узлу, которы находится в состоянии INFANT
 * Стать нормальным
 */
void LockFreeTree::helpInfant(LockFreeElement *node) {

}

/*
 * Разделение корня на два целевых узла
 * У дерева получается новый корень
 */
void LockFreeTree::SplitRoot(LockFreeElement *root, short int sepKey, LockFreeElement *n1,
               LockFreeElement *n2) {

}

/*
 * Слияние двух узлов, так что остаётся только один корень
 */
void LockFreeTree::MergeRoot(LockFreeElement *root, LockFreeElement *posibleNewRoot,
               LockFreeElement * c1, short int c1Key) {

}

/*
 * Замена в целевом чанке записи с данным ключом на neww, но если
 * нынешнее значение равно exp
 */
bool LockFreeTree::ReplaceInChunk(Chunk *chunk, short int key, LockFreeElement *exp,
                    LockFreeElement *neww) {

    return false;
}

/*
 * TODO
 * Я пока не понимаю, что делает эта функция
 */
void LockFreeTree::StabilizeChunk(Chunk *chunk) {

}

/*
 * TODO
 * Я пока не понимаю, что делает эта функция
 */
void LockFreeTree::MarkChunkFrozen(Chunk *chunk) {

}

/*
 * Выбор действия, которые надо воспроизвести с чанком, вышел ли он за пределы значений MAX и MIN
 */
short int LockFreeTree::FreezeDecision(Chunk *chunk) {

    return 0;
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

    return nullptr;
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

    return nullptr;
}

// те, которые не известны
/*
 * Поиск узла в данном чанке с ключом равному или
 * записи, ключ которой больше заданного, но минимальный среди таких
 * Результат должен записывать в глобальную переменную *cur,
 * что я считаю бредом, мб стоит (TODO) создать структуру , которую будет возвращать этот метод
 * Также записывает в глобальную переменные **prev и *next записи, которые идут до и после
 * cur, если бы список был упорядочен
 * Почему **prev, а не *prev:
 * TODO
 */
void LockFreeTree::Find(Chunk *chunk, short int key) {

}

/*
 * Возврат нормального указателя * вместо **
 */
Entry* LockFreeTree::EntPrt(Entry **entry) {

    return nullptr;
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
bool LockFreeTree::InsertToChunk(Chunk *chunk, short int key, LockFreeElement *data) {

    return false;
}

/*
 * Удаление в данном чанке записи с данным ключом
 */
bool LockFreeTree::DeleteInChunk(Chunk *chunk, short int key) {

    return false;
}

/*
 * Выдаёт максимальный ключ в списке
 */
short int LockFreeTree::getMaxKey(LockFreeElement *node) {

    return 0;
}

/*
 * TODO Не полностью понимаю, что делаем метод
 * Но судя по всему просто берёт ключ и ссылку на узел и делает запись, которая содержит их
 */
Entry* LockFreeTree::combine(short int key, LockFreeElement *node) {

    return nullptr;
}

/*
 * Создание нового пустого узла
 */
LockFreeElement* LockFreeTree::Allocate() {
    LockFreeElement *result = new LockFreeElement();
    nodes.push_back(result);
    return nodes;
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

// TODO не понятно пока что делать с этой функцией
bool LockFreeTree::isFrozen(Entry *entry) {

    return false;
}

// TODO не понятно пока что делать с этой функцией
bool LockFreeTree::isDeleted(Entry *entry) {

    return false;
}

// TODO не понятно пока что делать с этой функцией
void LockFreeTree::InsertEntry(Chunk *chunk, Entry *e, short int key) {

}

// TODO не понятно пока что делать с этой функцией
void LockFreeTree::MarkFrozen() {

}

// TODO не понятно пока что делать с этой функцией
void LockFreeTree::clearFrozen() {

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