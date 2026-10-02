cmake_minimum_required(VERSION 3.15)

project(IndexIt VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Ensure unicode path handling for Windows APIs
add_compile_definitions(UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS)
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -municode")

set(SOURCES
    src/main.cpp
    src/core/winfs.c
    src/core/extractor.c
    src/core/tokenizer.c
    src/index/trie.c
    src/index/hashtable.c
    src/index/inverted_index.c
    src/IExtractor.cpp
    src/IRanker.cpp
    src/ITermIndex.cpp
    src/CppInvertedIndex.cpp
    src/db.cpp
    src/UI.cpp
)

add_executable(indexit ${SOURCES})

target_include_directories(indexit PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/core
    ${PROJECT_SOURCE_DIR}/src/index
    ${PROJECT_SOURCE_DIR}/include
)

enable_testing()

add_executable(test_runner
    tests/test_runner.c
)

target_include_directories(test_runner PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/include
)

add_test(NAME IndexItTests COMMAND test_runner)
