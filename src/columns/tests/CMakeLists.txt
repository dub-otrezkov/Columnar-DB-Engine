add_executable(operators_test tests.h sum_test.cpp vertical_sum_test.cpp)

target_link_libraries(operators_test GTest::gtest GTest::gtest_main)
target_link_libraries(operators_test nodes_lib)