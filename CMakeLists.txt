cmake_minimum_required(VERSION 3.7)
project(Diplom)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

set(SOURCE_FILES Sources/main.cpp Headers/ITree.h Headers/BPlusTree/BPlusTree.h Headers/BPlusTree/BPlusElement.h Headers/TreeFactory.h Headers/Processing.h Sources/BPlusTree/BPlusTree.cpp Sources/TreeFactory.cpp Sources/BPlusTree/BPlusElement.cpp Sources/Processing.cpp)
add_executable(Diplom ${SOURCE_FILES})