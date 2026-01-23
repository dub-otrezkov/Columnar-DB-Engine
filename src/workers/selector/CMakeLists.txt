add_library(selector_worker_lib STATIC selector.h selector.cpp errors.h)

target_link_libraries(selector_worker_lib PUBLIC iworkers_lib io_worker_lib csv_reader_lib)