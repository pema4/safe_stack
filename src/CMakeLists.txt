add_library(
    safe_stack
    safe_stack.cpp
)

target_include_directories(
    safe_stack
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
