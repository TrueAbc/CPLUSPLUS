
include_directories(
        ${PROJECT_SOURCE_DIR}/includes
)

add_executable(test_lars lars_reactor.cpp)

target_link_libraries(test_lars
        ${SERVER_CLASS}
        ${BUF_TOOL}
        ${EVENT_TOOL}
        )
