cmake_minimum_required(VERSION 3.15)

project(IndexIt VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_definitions(UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS)

set(COMMON_INCLUDES
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/core
    ${PROJECT_SOURCE_DIR}/src/index
    ${PROJECT_SOURCE_DIR}/src/query
    ${PROJECT_SOURCE_DIR}/src/benchmark
    ${PROJECT_SOURCE_DIR}/src/dataset
)

# Core engine library (DJ_WORK ingestion + PR#5 indexing + C++ OOP layer + UI)
add_library(indexit_core STATIC
    src/core/winfs.c
    src/core/extractor.c
    src/core/tokenizer.c
    src/IExtractor.cpp
    src/index/hashtable.c
    src/index/trie.c
    src/index/inverted_index.c
    src/ITermIndex.cpp
    src/IRanker.cpp
    src/CppInvertedIndex.cpp
    src/UI.cpp
)
target_include_directories(indexit_core PUBLIC ${COMMON_INCLUDES})

# Main CLI executable (Teammates' CLI + dataset indexer + UI bridge)
add_executable(indexit
    src/main.c
    src/dataset/dataset.c
    src/query/query.c
    src/benchmark/benchmark.c
    src/index/index.c
)
target_include_directories(indexit PRIVATE ${COMMON_INCLUDES})
target_link_libraries(indexit PRIVATE indexit_core)

enable_testing()

# Teammates' test runner
add_executable(test_runner
    tests/test_runner.c
    src/query/query.c
    src/benchmark/benchmark.c
    src/index/index.c
)
target_include_directories(test_runner PRIVATE ${COMMON_INCLUDES})
add_test(NAME IndexItTests COMMAND test_runner)

# Reconciled full engine test runner
add_executable(test_engine
    tests/test_engine.cpp
)
target_include_directories(test_engine PRIVATE ${COMMON_INCLUDES})
target_link_libraries(test_engine PRIVATE indexit_core)
add_test(NAME EngineTests COMMAND test_engine)
