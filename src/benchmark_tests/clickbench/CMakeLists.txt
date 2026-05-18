add_executable(clickbench main.cpp)

target_link_libraries(clickbench tcmalloc engine_lib csv_reader_lib csv_writer_lib errors_lib nodes_lib iworkers_lib io_worker_lib exec_lib)

# AddressSanitizer / LeakSanitizer variant.
# Build:  cmake -DCLICKBENCH_ASAN=ON ... && make clickbench_asan
# Run:    ASAN_OPTIONS=detect_leaks=1:abort_on_error=0 LSAN_OPTIONS=print_suppressions=0 ./clickbench_asan
# tcmalloc is incompatible with ASan, so we strip it from the inherited link libs.
option(CLICKBENCH_ASAN "Build clickbench_asan with AddressSanitizer + LeakSanitizer" OFF)

if(CLICKBENCH_ASAN)
    add_executable(clickbench_asan main.cpp)

    target_link_libraries(clickbench_asan
        engine_lib csv_reader_lib csv_writer_lib errors_lib nodes_lib
        iworkers_lib io_worker_lib exec_lib)

    target_compile_options(clickbench_asan PRIVATE
        -O1
        -fsanitize=address
        -fsanitize=undefined
        -fno-omit-frame-pointer
        -fno-optimize-sibling-calls)

    target_link_options(clickbench_asan PRIVATE
        -fsanitize=address
        -fsanitize=undefined)

    get_target_property(_clickbench_asan_libs clickbench_asan LINK_LIBRARIES)
    if(_clickbench_asan_libs)
        list(REMOVE_ITEM _clickbench_asan_libs tcmalloc)
        set_target_properties(clickbench_asan PROPERTIES LINK_LIBRARIES "${_clickbench_asan_libs}")
    endif()
endif()
