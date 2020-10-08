enable_testing()

add_subdirectory(
    "${PROJECT_SOURCE_DIR}/extern/googletest"
    "extern/googletest"
)

add_executable(
    tests
    safe_stack_test.cpp
)

target_include_directories(
    tests
    PUBLIC
    ${PROJECT_SOURCE_DIR}/extern/googletest/googletest/include
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(
    tests
    gtest
    gtest_main
)

gtest_discover_tests(tests)
