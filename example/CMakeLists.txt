
include_directories(
        ${PROJECT_SOURCE_DIR}/includes
)

add_executable(test_lars lars_reactor.cpp)
add_executable(test_client client.cpp)

add_executable(test_udp_client udp_client_test.cpp)
add_executable(test_udp_server udp_test.cpp)

target_link_libraries(test_lars
        ${SERVER_CLASS}
        ${CONN_MANAGER}
        ${CONF_PARSER}
        ${SERVER_CLASS}
        )

target_link_libraries(test_client
        ${TCP_CLIENT})

target_link_libraries(test_udp_server
        ${UDP_SERVER}
        ${CONF_PARSER})

target_link_libraries(
        test_udp_client
        ${UDP_CLIENT}
)
