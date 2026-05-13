add_library(iworkers_lib STATIC base.h base.cpp)

target_link_libraries(iworkers_lib PUBLIC perf_stats_lib logger_lib)

add_subdirectory(io)
add_subdirectory(groupby)
add_subdirectory(orderby)
add_subdirectory(global_agregations)
add_subdirectory(ao_engine)
add_subdirectory(filters)