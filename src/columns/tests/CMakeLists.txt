add_executable(operators_test tests.h sum_test.cpp vertical_sum_test.cpp filter_test.cpp)
add_executable(sort_test tests.h order_test.cpp)

target_link_libraries(operators_test GTest::gtest GTest::gtest_main)
target_link_libraries(operators_test nodes_lib)
target_link_libraries(sort_test GTest::gtest GTest::gtest_main)
target_link_libraries(sort_test nodes_lib)