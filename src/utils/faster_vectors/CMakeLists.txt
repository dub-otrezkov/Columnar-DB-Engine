add_library(vectors_lib STATIC vector_string_2d.h vector_1d.h gstring.h string_heap.h vector_1d.cpp)
target_link_libraries(vectors_lib compress_lib)

add_subdirectory(tests)