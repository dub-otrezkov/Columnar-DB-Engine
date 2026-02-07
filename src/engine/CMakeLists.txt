add_library(engine_lib STATIC 
    engine.h
    engine.cpp
)

add_subdirectory(debug)
add_executable(engine_test test.cpp workers_test.cpp)

target_link_libraries(
    engine_lib
    csv_reader_lib
    csv_writer_lib
    errors_lib
    nodes_lib
    iworkers_lib
    io_worker_lib
    selector_worker_lib
)

target_link_libraries (engine_test GTest::gtest GTest::gtest_main)
target_link_libraries (engine_test engine_lib)