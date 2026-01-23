add_library(io_worker_lib STATIC io.h io.cpp errors.h)

target_link_libraries(io_worker_lib PUBLIC iworkers_lib csv_reader_lib)