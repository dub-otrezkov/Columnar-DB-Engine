cmake_minimum_required(VERSION 3.12)

set(CMAKE_BUILD_TYPE Release)

project(columnar_eng)

add_compile_options(-O3 -march=native -Wno-unused -fno-omit-frame-pointer)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

find_package(GTest REQUIRED)
find_package(Boost 1.81 REQUIRED)
include_directories(${GTEST_INCLUDE_DIR})

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(utils)

link_libraries(cint_lib Boost::boost)

add_subdirectory(csvio)
add_subdirectory(benchmark_tests)
add_subdirectory(columns)
add_subdirectory(ios_factory)
add_subdirectory(workers)
add_subdirectory(executor)
add_subdirectory(memory)
add_subdirectory(engine)
add_subdirectory(cli)
