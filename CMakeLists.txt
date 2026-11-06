cmake_minimum_required(VERSION 3.10)
project(ParasPhase3AdvancedAddons CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(paras_phase3_advanced
    src/DocumentStats.cpp
    src/SearchAnalytics.cpp
    src/ResultDeduplicator.cpp
    src/RankingMetrics.cpp
    src/PathUtilities.cpp
)

target_include_directories(paras_phase3_advanced PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(paras_phase3_advanced_tests tests/test_main.cpp)
target_link_libraries(paras_phase3_advanced_tests paras_phase3_advanced)

enable_testing()
add_test(NAME ParasPhase3AdvancedTests COMMAND paras_phase3_advanced_tests)
