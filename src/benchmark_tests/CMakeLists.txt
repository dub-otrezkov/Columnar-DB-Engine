add_executable(bench_test tests.cpp)

target_link_libraries(bench_test exec_lib)
target_link_libraries(bench_test GTest::gtest GTest::gtest_main)

add_subdirectory(clickbench)