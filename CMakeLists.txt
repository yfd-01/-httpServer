cmake_minimum_required(VERSION 3.16)
# set(CMAKE_BUILD_TYPE “Debug”)

project(httpServer)

set(CMAKE_CXX_STANDARD 20)
set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(LIB_DIR ${PROJECT_SOURCE_DIR}/lib)

add_library(buffer STATIC ${SRC_DIR}/buffer/buffer.cpp)

add_library(httpConn STATIC ${SRC_DIR}/http/httpConn.cpp)
add_library(httpRequest STATIC ${SRC_DIR}/http/httpRequest.cpp)
add_library(httpResponse STATIC ${SRC_DIR}/http/httpResponse.cpp)

add_library(logger STATIC ${SRC_DIR}/logger/logger.cpp)
add_library(devices STATIC ${SRC_DIR}/logger/devices.cpp)

add_library(heapTimer STATIC ${SRC_DIR}/timer/heapTimer.cpp)

add_library(sqlConnPool STATIC ${SRC_DIR}/pool/sqlConnPool.cpp)
add_library(threadPool STATIC ${SRC_DIR}/pool/threadPool.cpp)

add_library(epoller STATIC ${SRC_DIR}/server/epoller.cpp)
add_library(server STATIC ${SRC_DIR}/server/server.cpp)

add_executable(${PROJECT_NAME} ${SRC_DIR}/main.cpp)

target_link_libraries(logger devices)
target_link_libraries(httpConn httpRequest httpResponse buffer ${LIB_DIR}/libmysqlclient.so)
target_link_libraries(server sqlConnPool threadPool epoller heapTimer httpConn logger)
target_link_libraries(${PROJECT_NAME} server)
