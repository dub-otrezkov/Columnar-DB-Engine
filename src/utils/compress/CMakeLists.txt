add_library(compress_lib INTERFACE bitpack.h dict.h)
target_link_libraries(compress_lib INTERFACE cint_lib)
add_subdirectory(tests)
