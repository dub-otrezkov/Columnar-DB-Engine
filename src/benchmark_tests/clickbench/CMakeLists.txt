# Build modes are mutually exclusive (separate cmake build dirs):
#   Default:  builds `clickbench` with tcmalloc.
#   ASan:     cmake -DBUILD_ASAN=ON ... && make clickbench_asan
#             ASAN_OPTIONS=detect_leaks=1:abort_on_error=0 ./clickbench_asan

if(BUILD_ASAN)
    add_executable(clickbench_asan main.cpp)
    target_link_libraries(clickbench_asan
        engine_lib csv_reader_lib csv_writer_lib errors_lib nodes_lib
        iworkers_lib io_worker_lib exec_lib)
else()
    add_executable(clickbench main.cpp)
    target_link_libraries(clickbench
        tcmalloc engine_lib csv_reader_lib csv_writer_lib errors_lib nodes_lib
        iworkers_lib io_worker_lib exec_lib)
endif()
