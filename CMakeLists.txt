cmake_minimum_required(VERSION 3.20)
project(order_matching_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fsanitize=address,undefined -fno-omit-frame-pointer")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-fsanitize=address,undefined")
set(CMAKE_CXX_FLAGS_TSAN "-g -O1 -fsanitize=thread -fno-omit-frame-pointer")
set(CMAKE_EXE_LINKER_FLAGS_TSAN "-fsanitize=thread")

add_compile_options(-Wall -Wextra -Wpedantic)

include(cmake/CPM.cmake)

find_package(Threads REQUIRED)

CPMAddPackage(
    NAME googletest
    GITHUB_REPOSITORY google/googletest
    GIT_TAG v1.15.2
    OPTIONS "INSTALL_GTEST OFF" "gtest_force_shared_crt ON"
)

CPMAddPackage(
    NAME benchmark
    GITHUB_REPOSITORY google/benchmark
    GIT_TAG v1.9.1
    OPTIONS "BENCHMARK_ENABLE_TESTING OFF" "BENCHMARK_ENABLE_INSTALL OFF"
)

add_library(ome_core INTERFACE)
target_include_directories(ome_core INTERFACE ${CMAKE_SOURCE_DIR}/include)
target_compile_features(ome_core INTERFACE cxx_std_20)

# --- main executable
add_executable(ome_main src/main.cpp src/order_book.cpp)
target_link_libraries(ome_main PRIVATE ome_core)

# --- testy
enable_testing()
add_executable(ome_tests tests/test_order.cpp tests/test_order_book.cpp tests/test_memory_pool.cpp tests/test_pool_allocator.cpp tests/test_order_queue.cpp tests/test_spsc_queue.cpp tests/test_flat_order_book.cpp src/order_book.cpp)
target_link_libraries(ome_tests PRIVATE ome_core gtest_main Threads::Threads)
if(CMAKE_BUILD_TYPE STREQUAL "TSAN")
    add_test(NAME ome_tests COMMAND ome_tests)
else()
    include(GoogleTest)
    gtest_discover_tests(ome_tests)
endif()

# --- benchmarki
add_executable(ome_benchmarks benchmarks/bench_order_book.cpp src/order_book.cpp)
target_link_libraries(ome_benchmarks PRIVATE ome_core benchmark::benchmark)