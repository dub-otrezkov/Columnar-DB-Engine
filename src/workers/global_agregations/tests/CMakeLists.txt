add_executable(agregator_test agregations_test.h sum_test.cpp)

target_link_libraries(agregator_test GTest::gtest GTest::gtest_main)
target_link_libraries(agregator_test agregator_lib engine_lib)