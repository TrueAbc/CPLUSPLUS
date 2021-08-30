cmake_minimum_required(VERSION 3.20)
project(Lars)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

set(CMAKE_CXX_STANDARD 14)
set(SERVER_CLASS tcp_server)
set(BUF_TOOL buf_tool)
set(EVENT_TOOL kqueue)
set(CONN_MANAGER tcp_conn)

set(TCP_CLIENT tcp_client)

SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/libs)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

set(CMAKE_MACOSX_RPATH 1)
set(CMAKE_INSTALL_RPATH "${PROJECT_SOURCE_DIR}/libs")

include_directories(
        ./includes/)

link_directories(
        ${PROJECT_SOURCE_DIR}/libs
)

add_library(${BUF_TOOL} SHARED
        src/io_buf.cpp
        src/reactor_buf.cpp
        src/buf_pool.cpp)

add_library(
        ${EVENT_TOOL} SHARED
        src/event_loop.cpp
)

add_library(
    ${TCP_CLIENT} SHARED
        src/tcp_client.cpp
)

add_library(
        ${CONN_MANAGER} SHARED
        src/tcp_conn.cpp
)

add_library(${SERVER_CLASS} SHARED
        src/tcp_server.cpp)

add_subdirectory(
        ${PROJECT_SOURCE_DIR}/example
)

target_link_libraries(
        ${TCP_CLIENT}
        ${BUF_TOOL}
        ${EVENT_TOOL}
)

target_link_libraries(
        ${TCP_CONN}
        ${BUF_TOOL}
        ${EVENT_TOOL}
)

target_link_libraries(
        ${CONN_MANAGER}
        ${BUF_TOOL}
        ${EVENT_TOOL}
)

# server
target_link_libraries(
        ${SERVER_CLASS}
        ${CONN_MANAGER}
)