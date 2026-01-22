cmake_minimum_required(VERSION 4.1)

project(columnar_eng)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIR})

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(utils)

link_libraries(cint_lib logger_lib)

add_subdirectory(table_node)
add_subdirectory(engine)

