
include_directories(
        ./
)

add_executable(
        qps_server qps_server.cpp echoMessage.pb.cc
)

add_executable(
        qps_client qps_client.cpp echoMessage.pb.cc
)

target_link_libraries(
        qps_client
        protobuf
        ${TCP_CLIENT}
)

target_link_libraries(
        qps_server
        protobuf
        ${CONF_PARSER}
        ${SERVER_CLASS}
)


