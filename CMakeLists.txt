cmake_minimum_required(VERSION 3.15)

project(IndexIt VERSION 0.1.0 LANGUAGES C)

# Use a modern C standard.
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Main IndexIt executable.
add_executable(indexit
    src/main.c
)

target_include_directories(indexit
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
)

# Lightweight assert-based tests.
enable_testing()

add_executable(test_runner
    tests/test_runner.c
)

target_include_directories(test_runner
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
)

add_test(
    NAME IndexItTests
    COMMAND test_runner
)
