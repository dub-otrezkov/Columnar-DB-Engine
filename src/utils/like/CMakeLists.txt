add_library(like_lib INTERFACE kmp_stupid.h sse_stupid.h combined.h)

add_executable(like_bench benchmark.cpp)
target_link_libraries(like_bench PRIVATE like_lib)
