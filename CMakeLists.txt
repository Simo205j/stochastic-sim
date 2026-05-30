cmake_minimum_required(VERSION 3.25)

project(stochastic_sim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(stochastic INTERFACE)

target_include_directories(stochastic INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

enable_testing()

include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.4
)

FetchContent_MakeAvailable(Catch2)

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

add_executable(print_network
    examples/print_network.cpp
)

target_link_libraries(print_network
    PRIVATE
        stochastic
)

include(CTest)
include(Catch)
catch_discover_tests(stochastic_tests)
