cmake_minimum_required(VERSION 3.20)
project(test_lars)

include_directories(../includes)
set(SERVER_CLASS tcp_server)

add_library(${SERVER_CLASS} SHARED
        ../src/tcp_server.cpp)

add_executable(test_lars lars_reactor.cpp)

target_link_libraries(test_lars
        ${SERVER_CLASS}
        )