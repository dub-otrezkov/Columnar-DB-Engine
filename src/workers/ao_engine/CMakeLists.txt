add_library(agregator_engine_lib STATIC
    engine.h
    engine.cpp
    operators.h
    operators.cpp
    agregations.h
    agregations.cpp
)

target_link_libraries(agregator_engine_lib PUBLIC logger_lib)