cmake_minimum_required(VERSION 3.10)
project(IndexItAddons LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(indexit_addons
    DelimitedTextReader.cpp
    DocumentStats.cpp
    FileTypeClassifier.cpp
    MetadataFilter.cpp
    PathUtilities.cpp
    QueryParser.cpp
    QueryValidator.cpp
    RankingMetrics.cpp
    ResultDeduplicator.cpp
    ResultExporter.cpp
    SearchAnalytics.cpp
    SearchSuggestionEngine.cpp
    SnippetBuilder.cpp
    StringUtilities.cpp)
target_include_directories(indexit_addons PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(indexit_addons_tests test_main.cpp)
target_link_libraries(indexit_addons_tests PRIVATE indexit_addons)

add_executable(indexit_addons_demo demo.cpp)
target_link_libraries(indexit_addons_demo PRIVATE indexit_addons)

enable_testing()
add_test(NAME IndexItAddonsTests COMMAND indexit_addons_tests)
