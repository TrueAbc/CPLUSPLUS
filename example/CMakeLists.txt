
include_directories(
        ${PROJECT_SOURCE_DIR}/includes
)

add_executable(test_lars lars_reactor.cpp)
add_executable(test_client client.cpp)

target_link_libraries(test_lars
        ${SERVER_CLASS}
        ${CONN_MANAGER}
        ${SERVER_CLASS}
        )

target_link_libraries(test_client
        ${TCP_CLIENT})

