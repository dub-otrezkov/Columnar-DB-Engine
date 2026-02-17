add_library(agregator_lib STATIC
    agregator.h
    agregator.cpp
    operators.h
    operators.cpp
)

target_link_libraries(agregator_lib io_worker_lib)

add_subdirectory(tests)