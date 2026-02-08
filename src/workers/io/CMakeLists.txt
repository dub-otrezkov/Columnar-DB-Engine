add_library(io_worker_lib STATIC io.h io.cpp)

target_link_libraries(io_worker_lib PUBLIC iworkers_lib csv_reader_lib nodes_lib)