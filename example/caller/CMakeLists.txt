# 当前目录下的 main/调用代码
set(CONSUMER_SRC
    userinfo.cpp
    rpcclient.cpp
)

# Protobuf
file(GLOB PB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../*.pb.cc)

# 只列出当前 consumer 依赖的通用模块（可根据实际裁剪）
set(COMMON_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/../user.pb.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../usersqlite.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../Connection.cpp
    
)

add_executable(consumer1
    main.cpp
    ${CONSUMER_SRC}
    ${PB_SRC}
    ${COMMON_SRC}
)
add_executable(consumer2
    main2.cpp
    ${CONSUMER_SRC}
    ${PB_SRC}
    ${COMMON_SRC}
)

target_link_libraries(consumer1 mprpc protobuf sqlite3 mysqlclient)
target_link_libraries(consumer2 mprpc protobuf sqlite3 mysqlclient)

