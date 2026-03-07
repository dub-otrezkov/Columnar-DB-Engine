add_executable(exec_test tests.h tests.cpp)
add_executable(exec_big_test tests.h bigtest.cpp)

target_link_libraries(exec_test exec_lib)
target_link_libraries(exec_test GTest::gtest GTest::gtest_main)
target_link_libraries(exec_big_test exec_lib)
target_link_libraries(exec_big_test GTest::gtest GTest::gtest_main)