cmake_minimum_required(VERSION 3.10)

project(
    SafeStack 
    VERSION 0.1
    DESCRIPTION "Safe stack library"
    LANGUAGES CXX
)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic")
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(src)
add_subdirectory(app)

option(PACKAGE_TESTS "Build the tests" ON)
if(PACKAGE_TESTS)
    include(GoogleTest)
    enable_testing()
    add_subdirectory(test)
endif()

# todo: check if available, run only in RELEASE
find_package(Doxygen)
add_subdirectory(docs)
