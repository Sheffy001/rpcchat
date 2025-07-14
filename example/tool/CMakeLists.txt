aux_source_directory(. SRC_LIST)
add_library(tools ${SRC_LIST})
target_link_libraries(tools mprpc protobuf mysqlclient hiredis sqlite3)