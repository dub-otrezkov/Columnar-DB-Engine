cmake_minimum_required(VERSION 3.12)

project(columnar_eng)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIR})

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(utils)

link_libraries(cint_lib logger_lib)

add_subdirectory(csvio)
add_subdirectory(columns)
add_subdirectory(ios_factory)
add_subdirectory(workers)
add_subdirectory(executor)
add_subdirectory(engine)

