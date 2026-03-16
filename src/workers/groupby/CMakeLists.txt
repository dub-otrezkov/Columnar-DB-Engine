add_library(groupby_lib STATIC
    groupby.h
    groupby.cpp
    narrow_io.h
    narrow_io.cpp
)

target_link_libraries(groupby_lib io_worker_lib agregator_engine_lib)

add_subdirectory(tests)