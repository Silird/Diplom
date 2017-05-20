#ifndef DIPLOM_LOCKFREETREE_H
#define DIPLOM_LOCKFREETREE_H

#include <sudo_plugin.h>
#include <vector>
#include "../ITree.h"
#include "LockFreeElement.h"

// recovery Types
const short int RT_COPY = 0;
const short int RT_MERGE = 1;
const short int RT_SPLIT = 2;

// trigger Types
const short int TT_DELETE = 0;
const short int TT_INSERT = 1;
const short int TT_REPLACE = 2;
const short int TT_ENSLAVE = 4;


class LockFreeTree : public virtual ITree {
private:
    std::vector<LockFreeElement*> nodes;
    int MAX, MIN;
    LockFreeElement *root;

public:
    LockFreeTree(int range);
    ~LockFreeTree();
    bool add(short int key);
    bool search(short int key);
    bool remove(short int key);
    void print();
    void printValues();

private:
    // те, которые известны
    /*
     * Находит лист, у которого ПОТЕНЦИАЛЬНО может находится данный ключ
     */
    LockFreeElement* FindLeaf(short int key);

    /*
     * Находит родительский узел, в котором содержится пара key и childNode
     * Записвает в prntEnt запись целевого узла
     * Если slaveEnt не нул, то записывает в него потенциального раба для мерджа
     */
    LockFreeElement* FindParent(short int key, LockFreeElement *childNode,
                                Entry **prntEnt, Entry **slaveEnt);

    /*
     * Вставляет в дерево вместо узла node, два разделённых узла
     * Первый узел содержится в поле neww у node, а второй в поле nextNew у neww
     * Первый содержит меньшую часть ключей, второй большую
     * sepKey - ключ для второго узла.
     */
    void InsertSplitNodes(LockFreeElement *node, short int sepKey);

    /*
     * Находит раба для мёрджа с master
     * Устанавливет состояния у обоих узлов
     */
    LockFreeElement* FindJoinSlave(LockFreeElement *master);

    /*
     * Пытается установить отношение master-slave
     * Может произойти смена ролей
     */
    bool SetSlave(LockFreeElement *master, LockFreeElement *slave, short int masterKey,
                  short int slaveKey);

    /*
     * Устанавливает вместо двух узлов между которыми установлено отношение master-slave
     * два новых узла.
     * Первый узел содержится в поле neww у master, а второй в поле nextNew у neww
     * slave находится в поле joinBuddy у master
     */
    void InsertBorrowNodes(LockFreeElement *master, short int sepKey);

    /*
     * Устанавливает вместо двух узлов между которыми установлено отношение master-slave
     * один новый узл.
     * Узел содержится в поле neww у master
     * slave находится в поле joinBuddy у master
     */
    void InsertMergeNode(LockFreeElement *master);

    /*
     * Попытка обновление дерева
     * freezeState - состояние узла
     * node - узел, который хочет обновить дерево
     * sepKey - возможный ключ сепаратор, если сплит или борроу
     */
    void CallForUpdate(short int freezeState, LockFreeElement *node, short int sepKey);

    /*
     * Помощь узлу, которы находится в состоянии INFANT
     * Стать нормальным
     */
    void helpInfant(LockFreeElement *node);

    /*
     * Разделение корня на два целевых узла
     * У дерева получается новый корень
     */
    void SplitRoot(LockFreeElement *root, short int sepKey, LockFreeElement *n1,
                   LockFreeElement *n2);

    /*
     * Слияние двух узлов, так что остаётся только один корень
     */
    void MergeRoot(LockFreeElement *root, LockFreeElement *posibleNewRoot,
                   LockFreeElement * c1, short int c1Key);

    /*
     * Замена в целевом чанке записи с данным ключом на neww, но если
     * нынешнее значение равно exp
     */
    bool ReplaceInChunk(Chunk *chunk, short int key, LockFreeElement *exp,
                        LockFreeElement *neww);

    /*
     * TODO
     * Я пока не понимаю, что делает эта функция
     */
    void StabilizeChunk(Chunk *chunk);

    /*
     * TODO
     * Я пока не понимаю, что делает эта функция
     */
    void MarkChunkFrozen(Chunk *chunk);

    /*
     * Выбор действия, которые надо воспроизвести с чанком, вышел ли он за пределы значений MAX и MIN
     */
    short int FreezeDecision(Chunk *chunk);

    /*
     * Замораживание узла
     * С поиском всех нужных узлов для мерджа например
     * Возвращает null если всё окки
     * Обозначение всех входных данных смотри в FreezeRecovery,
     * так как он является логическим продолжением
     */
    Chunk* Freeze(LockFreeElement *node, short int key, LockFreeElement *expected,
                  LockFreeElement *data, short int tgr, bool *res);

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
    Chunk* FreezeRecovery(LockFreeElement *oldNode, short int key,
                          LockFreeElement *expected, LockFreeElement *input,
                          short int recovType, LockFreeElement *mergePartner,
                          short int trigger, bool *result);

    // те, которые не известны
    /*
     * Поиск узла в данном чанке с ключом равному или
     * записи, ключ которой больше заданного, но минимальный среди таких
     * Результат должен записывать в глобальную переменную *cur,
     * что я считаю бредом, мб стоит создать структуру, которую будет возвращать этот метод
     * Также записывает в глобальную переменные **prev и *next записи, которые идут до и после
     * cur, если бы список был упорядочен
     * Почему **prev, а не *prev:
     * TODO
     */
    void Find(Chunk *chunk, short int key);

    /*
     * Возврат нормального указателя * вместо **
     */
    Entry* EntPrt(Entry **entry);

    /*
     * Поиск в данном чанке записи с ключом key
     */
    bool SearchInChunk(Chunk *chunk, short int key);

    /*
     * Вставка в чанк записи, которая содержит данный ключ и данную ссылку на узел
     */
    bool InsertToChunk(Chunk *chunk, short int key, LockFreeElement *data);

    /*
     * Удаление в данном чанке записи с данным ключом
     */
    bool DeleteInChunk(Chunk *chunk, short int key);

    /*
     * Выдаёт максимальный ключ в списке
     */
    short int getMaxKey(LockFreeElement *node);

    /*
     * TODO Не полностью понимаю, что делаем метод
     * Но судя по всему просто берёт ключ и ссылку на узел и делает запись, которая содержит их
     */
    Entry* combine(short int key, LockFreeElement *node);

    /*
     * Создание нового пустого узла
     */
    LockFreeElement* Allocate();

    /*
     * Добавление в рут два данных сына с данными ключами
     */
    void addRootSons(LockFreeElement *root, short int sepKey1, LockFreeElement *n1,
                     short int sepKey2, LockFreeElement *n2);

    /*
     * Выдаёт сколько записей содержится в чанке
     * Также записывает в firstEnt и secondEnt
     * ссылку на первую и вторую запись соотвественно
     * Если рассматривать список как упорядочный
     */
    int getEntNum(Chunk *chunk, Entry *firstEnt, Entry *secondEnt);

    // TODO не понятно пока что делать с этой функцией
    bool isFrozen(Entry *entry);

    // TODO не понятно пока что делать с этой функцией
    bool isDeleted(Entry *entry);

    // TODO не понятно пока что делать с этой функцией
    void InsertEntry(Chunk *chunk, Entry *e, short int key);

    // TODO не понятно пока что делать с этой функцией
    void MarkFrozen();

    // TODO не понятно пока что делать с этой функцией
    void clearFrozen();

    /*
     * Копирует содержимое чанка oldNode в newNode
     */
    void copyToOneChunkNode(LockFreeElement *oldNode, LockFreeElement* newNode);

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
    short int mergeToTwoNodes(LockFreeElement *oldNode, LockFreeElement *mergePartner,
                              LockFreeElement *newNode1, LockFreeElement *newNode2);

    /*
     * В чанк нового узла записываются все значения из oldNode и mergePartner соотвественно
     */
    void mergeToOneNode(LockFreeElement *oldNode, LockFreeElement *mergePartner,
                        LockFreeElement *newNode);

    /*
     * Разделение одного узла на два
     * Находится ключ-сепаратор - медиана ключей в oldNode
     * В первый узел записываются все записи с ключами меньше или равными, чем ключ-сепаратор
     * Во второй все записи, которые больше ключа-сепаратора
     */
    short int splitIntoTwoNodes(LockFreeElement *oldNode, LockFreeElement *newNode1,
                                LockFreeElement *newNode2);

    /*
     * Перемещение первой записи (если бы список был упорядочен) из first
     * в second
     */
    void moveEntryFromFirstToSecond(LockFreeElement *first, LockFreeElement *second);
};
#endif //DIPLOM_LOCKFREETREE_H
