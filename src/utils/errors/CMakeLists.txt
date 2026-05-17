add_library(errors_lib STATIC
    errors.cpp
    errors.h
    errors_templates.h
)

# target_include_directories(errors_lib PUBLIC
#     ${CMAKE_CURRENT_SOURCE_DIR}
# )