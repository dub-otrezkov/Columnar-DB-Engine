add_library(bitpack_lib INTERFACE)
target_sources(bitpack_lib INTERFACE bitpack.h)

add_subdirectory(tests)
