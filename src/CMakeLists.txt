cmake_minimum_required(VERSION 3.12)

set(CMAKE_BUILD_TYPE Release)

project(columnar_eng)

option(BUILD_ASAN "Build with AddressSanitizer + UBSan (drops tcmalloc, use a separate build dir)" OFF)

if(BUILD_ASAN)
    add_compile_options(-O1 -march=native -Wno-unused -fno-omit-frame-pointer -fno-optimize-sibling-calls -g
                        -fsanitize=address -fsanitize=undefined)
    add_link_options(-fsanitize=address -fsanitize=undefined)
else()
    add_compile_options(-O3 -march=native -Wno-unused -fno-omit-frame-pointer -g)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

find_package(GTest REQUIRED)
find_package(Boost 1.81 REQUIRED)
include_directories(${GTEST_INCLUDE_DIR})

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(utils)

if(BUILD_ASAN)
    link_libraries(cint_lib Boost::boost)
else()
    link_libraries(cint_lib Boost::boost tcmalloc)
endif()

add_subdirectory(csvio)
add_subdirectory(benchmark_tests)
add_subdirectory(columns)
add_subdirectory(ios_factory)
add_subdirectory(workers)
add_subdirectory(executor)
add_subdirectory(memory)
add_subdirectory(engine)
add_subdirectory(cli)
