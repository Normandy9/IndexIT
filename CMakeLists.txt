cmake_minimum_required(VERSION 3.10)
project(ParasPhase3Addons CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(paras_phase3_addons
    src/StringUtilities.cpp
    src/QueryParser.cpp
    src/MetadataFilter.cpp
    src/SnippetBuilder.cpp
    src/ResultExporter.cpp
)

target_include_directories(paras_phase3_addons PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(paras_phase3_addon_tests tests/test_main.cpp)
target_link_libraries(paras_phase3_addon_tests paras_phase3_addons)

enable_testing()
add_test(NAME ParasPhase3AddonTests COMMAND paras_phase3_addon_tests)
