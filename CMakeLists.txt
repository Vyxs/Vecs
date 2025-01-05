cmake_minimum_required(VERSION 3.29)
project(vecs)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

enable_testing()

add_executable(run_tests
    src/vecs/Entity.h
    src/vecs/SparseSet.h
    src/vecs/Pool.h
    src/vecs/ECS.h
    tests/test_vecs_basic_operation.cpp
)

target_link_libraries(run_tests GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(run_tests)