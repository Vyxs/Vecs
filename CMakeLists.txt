cmake_minimum_required(VERSION 3.29)
project(vecs)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)
FetchContent_MakeAvailable(googletest)
FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.6.0
)
FetchContent_MakeAvailable(benchmark)
FetchContent_Declare(
        entt
        GIT_REPOSITORY https://github.com/skypjack/entt.git
        GIT_TAG v3.12.2
)
FetchContent_MakeAvailable(entt)

enable_testing()

add_library(vecs INTERFACE)
target_include_directories(vecs INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(run_tests
    tests/test_vecs_basic_operation.cpp
    tests/test_vecs_view.cpp
    src/vecs/Entity.h
    src/vecs/SparseSet.h
    src/vecs/Pool.h
    src/vecs/ECS.h
    src/vecs/View.h
)

target_link_libraries(run_tests GTest::gtest_main)

add_executable(run_benchmarks
        src/benchmarks/benchmark_comparative_view.cpp
)

target_link_libraries(run_benchmarks PRIVATE
        vecs
        EnTT::EnTT
        benchmark::benchmark
        benchmark::benchmark_main
)

include(GoogleTest)
gtest_discover_tests(run_tests)