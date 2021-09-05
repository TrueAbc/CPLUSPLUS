cmake_minimum_required(VERSION 3.20)
project(Lars)

set(PROTOBUF_PATH /Users/trueabc/Envs/Protobuf)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 14)

set(SERVER_CLASS tcp_server)
set(BUF_TOOL buf_tool)
set(EVENT_TOOL kqueue)

set(THREAD_POOL thread_pool)

set(CONN_MANAGER tcp_conn)
set(TCP_CLIENT tcp_client)
set(CONF_PARSER conf_parser)

set(UDP_SERVER udp_server)
set(UDP_CLIENT udp_client)


SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/libs)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

set(CMAKE_MACOSX_RPATH 1)
set(CMAKE_INSTALL_RPATH "${PROJECT_SOURCE_DIR}/libs")


include_directories(
        ./includes/)

include_directories(
        ${PROTOBUF_PATH}/include
)

link_directories(
        ${PROTOBUF_PATH}/lib
)

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
        ${CONF_PARSER} SHARED
        src/config_file.cpp
)

add_library(
        ${THREAD_POOL} STATIC
        src/thread_pool.cpp
)

add_library(
    ${TCP_CLIENT} SHARED
        src/tcp_client.cpp
)

add_library(
        ${UDP_CLIENT} SHARED
        src/udp_client.cpp
)

add_library(
        ${UDP_SERVER} SHARED
        src/udp_server.cpp
)

## 两个库之间有相互引用, 不能是动态库
add_library(
        ${CONN_MANAGER} STATIC
        src/tcp_conn.cpp
)

add_library(${SERVER_CLASS} STATIC
        src/tcp_server.cpp
        )

add_subdirectory(
        ${PROJECT_SOURCE_DIR}/example
)

add_subdirectory(
        ${PROJECT_SOURCE_DIR}/test

)

target_link_libraries(
        ${TCP_CLIENT}
        ${BUF_TOOL}
        ${EVENT_TOOL}
)


target_link_libraries(
        ${CONF_PARSER}
)

target_link_libraries(
        ${UDP_CLIENT} ${EVENT_TOOL}
)

target_link_libraries(
        ${UDP_SERVER} ${EVENT_TOOL}
)

# 仅仅源文件包含了tcp_server
target_link_libraries(
        ${CONN_MANAGER}

        ${BUF_TOOL}
        ${EVENT_TOOL}
        ${SEVER_CLASS}

)

target_link_libraries(
        ${THREAD_POOL}
        ${EVENT_TOOL}
        ${CONN_MANAGER}
)

# server
target_link_libraries(
        ${SERVER_CLASS}

        ${THREAD_POOL}
        ${BUF_TOOL}
        ${EVENT_TOOL}

#        ${CONN_MANAGER}
)