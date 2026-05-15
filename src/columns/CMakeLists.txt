link_libraries(errors_lib vectors_lib iworkers_lib like_lib)

add_library(nodes_lib STATIC
    types/types.cpp
    types/types.h
    operators/operators.h
    operators/io_operators.h
    operators/arithmetic_operators.h
    operators/min_max.h
    operators/distinct.h
    operators/filter.h
    operators/vector_like.h
    operators/order.h
    operators/dates.h
    types/int_columns.cpp
    types/str_columns.cpp
    types/double_columns.cpp
    types/time_columns.cpp
)

add_subdirectory(tests)
