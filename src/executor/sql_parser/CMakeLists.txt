add_library(sql_parser_lib STATIC 
    tokenizer.h
    tokenizer.cpp
    from_token.cpp
    name_token.cpp
    create_token.cpp
    select_token.cpp
    where_token.cpp
    group_token.cpp
    order_token.cpp
    limit_token.cpp
    misc.cpp
)

target_link_libraries(
    sql_parser_lib
    errors_lib
    csv_reader_lib
    engine_lib
    ios_factory_lib
    agregator_lib
    agregator_engine_lib
    groupby_lib
    orderby_lib
    filters_lib
)