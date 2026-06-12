add_library(csv_reader_lib STATIC
    csv_reader.cpp
    csv_reader.h
)
target_link_libraries(csv_reader_lib PUBLIC mmap_input_lib)

add_library(csv_writer_lib STATIC
    csv_writer.cpp
    csv_writer.h
)
target_link_libraries(csv_writer_lib PUBLIC mmap_input_lib)

add_executable(csvio_test test.cpp)

target_link_libraries(csvio_test csv_reader_lib csv_writer_lib errors_lib vectors_lib)
target_link_libraries (csvio_test GTest::gtest GTest::gtest_main)
