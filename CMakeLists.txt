cmake_minimum_required(VERSION 3.25)

project(stochastic_sim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(stochastic INTERFACE)

target_include_directories(stochastic INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_package(Threads REQUIRED)

enable_testing()

include(FetchContent)

# --------------------
# Catch2
# --------------------

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.4
)

FetchContent_MakeAvailable(Catch2)

# --------------------
# Google Benchmark
# --------------------

option(BUILD_BENCHMARKS "Build benchmark executables" ON)

if(BUILD_BENCHMARKS)
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        google_benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.9.1
    )

    FetchContent_MakeAvailable(google_benchmark)
endif()

# --------------------
# Tests
# --------------------

add_executable(stochastic_tests
    tests/reaction_tests.cpp
    tests/symbol_table_tests.cpp
    tests/simulation_tests.cpp
)

target_link_libraries(stochastic_tests
    PRIVATE
        stochastic
        Catch2::Catch2WithMain
)

# --------------------
# Examples
# --------------------

add_executable(print_network
    examples/print_network.cpp
)

target_link_libraries(print_network
    PRIVATE
        stochastic
)

add_executable(abc_decay
    examples/abc_decay.cpp
)

target_link_libraries(abc_decay
    PRIVATE
        stochastic
)

add_executable(circadian_rhythm
    examples/circadian_rhythm.cpp
)

target_link_libraries(circadian_rhythm
    PRIVATE
        stochastic
)

add_executable(covid19
    examples/covid19.cpp
    examples/covid19_helper.cpp
)

target_link_libraries(covid19
    PRIVATE
        stochastic
)

add_executable(covid19_parallel
    examples/covid19_parallel.cpp
    examples/covid19_helper.cpp
)

target_link_libraries(covid19_parallel
    PRIVATE
        stochastic
        Threads::Threads
)

# --------------------
# Benchmarks
# --------------------

if(BUILD_BENCHMARKS)
    add_executable(covid19_benchmark
        benchmarks/covid19_benchmark.cpp
        examples/covid19_helper.cpp
    )

    target_link_libraries(covid19_benchmark
        PRIVATE
            stochastic
            benchmark::benchmark
            Threads::Threads
    )

    target_include_directories(covid19_benchmark
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/examples
    )
endif()

# --------------------
# Test discovery
# --------------------

include(CTest)
include(Catch)
catch_discover_tests(stochastic_tests)