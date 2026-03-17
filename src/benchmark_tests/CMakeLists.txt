add_executable(bench_test tests.cpp)
add_executable(bench_debug debug.cpp)

target_link_libraries(bench_test exec_lib)
target_link_libraries(bench_test GTest::gtest GTest::gtest_main)
target_link_libraries(bench_debug exec_lib)
target_link_libraries(bench_debug GTest::gtest GTest::gtest_main)