add_library(iworkers_lib STATIC base.h base.cpp)

add_subdirectory(io)
add_subdirectory(groupby)
add_subdirectory(orderby)
add_subdirectory(global_agregations)
add_subdirectory(agregations_engine)
add_subdirectory(filters)