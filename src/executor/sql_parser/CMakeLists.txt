add_library(sql_parser_lib STATIC 
    tokenizer.h
    tokenizer.cpp
    errors.h
    from_token.cpp
    name_token.cpp
    create_token.cpp
    select_token.cpp
)

target_link_libraries(sql_parser_lib errors_lib csv_reader_lib engine_lib ios_factory_lib)