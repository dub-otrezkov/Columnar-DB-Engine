add_library(groupby_lib STATIC
    groupby.h
    groupby.cpp
)

target_link_libraries(groupby_lib io_worker_lib hashmap_lib agregator_engine_lib perf_stats_lib)

add_subdirectory(tests)