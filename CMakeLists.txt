cmake_minimum_required(VERSION 3.15)

project(IndexIt VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(indexit
    src/main.c
    src/query/query.c
    src/benchmark/benchmark.c
    src/index/index.c
)

target_include_directories(indexit
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/src/query
        ${PROJECT_SOURCE_DIR}/src/benchmark
        ${PROJECT_SOURCE_DIR}/src/index
)

enable_testing()

add_executable(test_runner
    tests/test_runner.c
    src/query/query.c
    src/benchmark/benchmark.c
    src/index/index.c
)

target_include_directories(test_runner
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/src/query
        ${PROJECT_SOURCE_DIR}/src/benchmark
        ${PROJECT_SOURCE_DIR}/src/index
)

add_test(
    NAME IndexItTests
    COMMAND test_runner
)
