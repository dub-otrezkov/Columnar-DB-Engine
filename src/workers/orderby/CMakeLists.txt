add_library(orderby_lib STATIC
    orderby.h
    orderby.cpp
)

target_link_libraries(orderby_lib io_worker_lib)

add_subdirectory(tests)