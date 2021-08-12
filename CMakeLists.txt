cmake_minimum_required(VERSION 3.20)
project(Lars)

set(CMAKE_CXX_STANDARD 14)
set(SERVER_CLASS tcp_server)
set(BUF_TOOL buf_tool)

include_directories(
        ./includes/)

add_library(${SERVER_CLASS} SHARED
        src/tcp_server.cpp)

add_library(${BUF_TOOl} SHARED
        src/io_buf.cpp
        src/reactor_buf.cpp
        src/buf_pool.cpp)

add_executable(Lars main.cpp)

target_link_libraries(Lars
       ${SERVER_CLASS}
)

target_link_libraries(Lars
        ${BUF_TOOL})