cmake_minimum_required(VERSION 3.20)
# set(CMAKE_BUILD_TYPE “Debug”)

project(server)

set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)

add_library(sqlConnPool STATIC ${SRC_DIR}/pool/sqlConnPool.cpp)
add_library(threadPool STATIC ${SRC_DIR}/pool/threadPool.cpp)
add_library(heapTimer STATIC ${SRC_DIR}/timer/heapTimer.cpp)
# add_library(bd STATIC ${SRC_DIR}/logger/blockingdeque.h)

add_executable(${PROJECT_NAME} ${SRC_DIR}/main.cpp)
target_link_libraries(${PROJECT_NAME} libmysqlclient.so.21.2.32 sqlConnPool threadPool heapTimer)
