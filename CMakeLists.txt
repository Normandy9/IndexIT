cmake_minimum_required(VERSION 3.10)
project(IndexIt LANGUAGES C CXX)

if(WIN32)
    enable_language(RC)
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(indexit_core
    src/CppInvertedIndex.cpp
    src/IExtractor.cpp
    src/IRanker.cpp
    src/ITermIndex.cpp
    src/UI.cpp
    src/cache/BinaryIndexCache.cpp
    src/core/extractor.c
    src/core/tokenizer.c
    src/core/winfs.c
    src/db.cpp
    src/index/hashtable.c
    src/index/inverted_index.c
    src/index/trie.c
    third_party/miniz/miniz.c
    third_party/miniz/miniz_tdef.c
    third_party/miniz/miniz_tinfl.c
    third_party/sqlite3/sqlite3.c)

target_include_directories(indexit_core PUBLIC include src PRIVATE
    src/core
    third_party/miniz
    third_party/sqlite3)
target_link_libraries(indexit_core PUBLIC shell32)

add_executable(indexit src/main.cpp)
target_link_libraries(indexit PRIVATE indexit_core)

add_executable(indexit_gui WIN32 src/Gui.cpp src/IndexIt.rc)
target_link_libraries(indexit_gui PRIVATE indexit_core shell32 ole32 comctl32)
add_dependencies(indexit_gui indexit)
set_target_properties(indexit_gui PROPERTIES OUTPUT_NAME "IndexItWindow")

add_executable(test_runner tests/test_runner.cpp)
target_link_libraries(test_runner PRIVATE indexit_core)
file(TO_CMAKE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" INDEXIT_SOURCE_DIR)
target_compile_definitions(test_runner PRIVATE INDEXIT_SOURCE_DIR="${INDEXIT_SOURCE_DIR}")

enable_testing()
add_test(NAME IndexItTests COMMAND test_runner)
