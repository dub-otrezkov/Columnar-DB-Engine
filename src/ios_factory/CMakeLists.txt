add_library(ios_factory_lib STATIC 
    errors.h
    ios_factory.h
)

target_link_libraries(ios_factory_lib errors_lib)