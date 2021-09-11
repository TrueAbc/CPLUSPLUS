include_directories(
        ${PROJECT_SOURCE_DIR}/includes
)

add_executable(test_func main.cpp)

target_link_libraries(
        test_func
        ${EVENT_TOOL}
)