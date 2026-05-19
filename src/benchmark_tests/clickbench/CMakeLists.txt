if(BUILD_ASAN)
    add_executable(clickbench_asan main.cpp)
    target_link_libraries(clickbench_asan
        engine_lib csv_reader_lib csv_writer_lib errors_lib nodes_lib
        iworkers_lib io_worker_lib exec_lib)

    target_compile_definitions(clickbench_asan PRIVATE CLICKBENCH_NO_TCMALLOC)
else()
    add_executable(clickbench main.cpp)
    target_link_libraries(clickbench
        tcmalloc engine_lib csv_reader_lib csv_writer_lib errors_lib nodes_lib
        iworkers_lib io_worker_lib exec_lib)
endif()
