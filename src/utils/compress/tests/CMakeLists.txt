add_executable(bitpack_test bitpack_test.cpp)
target_link_libraries(bitpack_test compress_lib GTest::gtest GTest::gtest_main)

add_executable(dict_test dict_test.cpp)
target_link_libraries(dict_test compress_lib vectors_lib GTest::gtest GTest::gtest_main)

add_executable(delta_test delta_test.cpp)
target_link_libraries(delta_test compress_lib GTest::gtest GTest::gtest_main)
