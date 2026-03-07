add_library(filters_lib STATIC filter.h filter.cpp)

target_link_libraries(filters_lib io_worker_lib)

add_executable(filters_test test.cpp)

target_link_libraries(filters_test GTest::gtest GTest::gtest_main)
target_link_libraries(filters_test filters_lib engine_lib)
