add_executable(groupby_test test.cpp)

target_link_libraries(groupby_test GTest::gtest GTest::gtest_main)
target_link_libraries(groupby_test groupby_lib engine_lib agregator_engine_lib)
