link_libraries(errors_lib)

add_subdirectory(tests)

add_library(nodes_lib STATIC
    types.cpp
    types.h
    operators.h
    errors.h
)