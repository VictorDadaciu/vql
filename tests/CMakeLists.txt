cmake_minimum_required(VERSION 3.21..4.4)

add_executable(vql_tests test_runner.cpp)

target_link_libraries(vql_tests
    vql
    vtl
)

option(VQL_RUN_TESTS "Run tests after build" ${VQL_IS_TOP_LEVEL})
if (${VQL_RUN_TESTS})
    message("Will run tests after build")
    add_custom_target(
        vql_run_tests ALL
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/__dummy_file
    )
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/__dummy_file
        COMMENT "Running tests..."
        COMMAND ./../build/tests/vql_tests
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    add_dependencies(vql_run_tests vql_tests)
endif()
