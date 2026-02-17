add_subdirectory(sql_parser)
add_subdirectory(debug)
add_subdirectory(tests)

add_library(exec_lib STATIC executor.h executor.cpp)
target_link_libraries(exec_lib engine_lib csv_reader_lib csv_writer_lib errors_lib nodes_lib iworkers_lib io_worker_lib selector_worker_lib sql_parser_lib)