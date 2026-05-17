add_library(agregator_lib STATIC
    agregator.h
    agregator.cpp
)

target_link_libraries(agregator_lib io_worker_lib agregator_engine_lib perf_stats_lib)

add_subdirectory(tests)