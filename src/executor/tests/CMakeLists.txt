add_executable(exec_test tests.h tests.cpp)

target_link_libraries(exec_test exec_lib)
target_link_libraries(exec_test GTest::gtest GTest::gtest_main)