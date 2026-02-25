link_libraries(errors_lib)

add_library(nodes_lib STATIC
    types/types.cpp
    types/types.h
    operators/operators.h
    operators/io_operators.h
    operators/arithmetic_operators.h
    operators/min_max.h
    types/int_columns.cpp
    types/str_columns.cpp
    types/double_columns.cpp
    types/time_columns.cpp
)

add_subdirectory(tests)
