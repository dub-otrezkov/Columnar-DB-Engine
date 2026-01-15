add_library(engine_lib STATIC 
    engine.h
    engine.cpp
    table_input.cpp
    nodes_factory.h
    nodes_factory.cpp
)

add_executable(engine_test test.cpp)

target_link_libraries(engine_test engine_lib csv_reader_lib csv_writer_lib errors_lib nodes_lib)
target_link_libraries (engine_test GTest::gtest GTest::gtest_main)