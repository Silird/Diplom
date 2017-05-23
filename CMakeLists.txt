cmake_minimum_required(VERSION 3.7)
project(Diplom)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -mcx16")

set(SOURCE_FILES Sources/main.cpp Headers/ITree.h Headers/BPlusTree/BPlusTree.h Headers/BPlusTree/BPlusElement.h Headers/TreeFactory.h Headers/Processing.h Sources/BPlusTree/BPlusTree.cpp Sources/TreeFactory.cpp Sources/BPlusTree/BPlusElement.cpp Sources/Processing.cpp Headers/Lock-Free/Entry.h Headers/Lock-Free/Chunk.h Headers/Lock-Free/LockFreeElement.h Headers/Lock-Free/LockFreeTree.h Sources/Lock-Free/LockFreeTree.cpp Headers/Lock-Free/FindResult.h Headers/Lock-Free/NodeState.h Headers/Lock-Free/EntryDataKey.h Headers/Lock-Free/EntryNext.h)
add_executable(Diplom ${SOURCE_FILES})