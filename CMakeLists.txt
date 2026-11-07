cmake_minimum_required(VERSION 3.10)
project(ParasPhase3MoreAddons CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories(include)
add_library(paras_more_addons
    src/QueryValidator.cpp
    src/FileTypeClassifier.cpp
    src/SearchSuggestionEngine.cpp
    src/DelimitedTextReader.cpp)
add_executable(paras_addons_tests tests/test_main.cpp)
target_link_libraries(paras_addons_tests paras_more_addons)
add_executable(paras_addons_demo examples/demo.cpp)
target_link_libraries(paras_addons_demo paras_more_addons)
enable_testing()
add_test(NAME paras_addons_tests COMMAND paras_addons_tests)
