add_executable(bitpack_test bitpack_test.cpp)
target_link_libraries(bitpack_test bitpack_lib GTest::gtest GTest::gtest_main)
