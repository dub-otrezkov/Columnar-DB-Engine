add_executable(orderby_test test.cpp)

target_link_libraries(orderby_test GTest::gtest GTest::gtest_main)
target_link_libraries(orderby_test orderby_lib engine_lib)
