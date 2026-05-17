add_library(hashmap_lib INTERFACE hashmap.h hashset.h)

add_executable(hashmap_time_test time_test.cpp)

target_link_libraries(hashmap_time_test hashmap_lib)
target_link_libraries(hashmap_time_test GTest::gtest GTest::gtest_main)