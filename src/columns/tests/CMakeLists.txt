add_executable(operators_test sum_test.cpp)

target_link_libraries(operators_test GTest::gtest GTest::gtest_main)
target_link_libraries(operators_test nodes_lib)