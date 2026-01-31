add_library(sql_parser_lib STATIC tokenizer.h tokenizer.cpp errors.h)

target_link_libraries(sql_parser_lib errors_lib csv_reader_lib engine_lib)