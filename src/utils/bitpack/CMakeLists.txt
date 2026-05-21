add_library(bitpack_lib INTERFACE bitpack.h)
target_link_libraries(bitpack_lib INTERFACE cint_lib)
add_subdirectory(tests)
