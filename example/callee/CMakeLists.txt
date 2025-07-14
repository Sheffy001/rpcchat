# 通用的公共源文件（用于三个服务）
file(GLOB COMMON_SRC
    ../Connection.cpp
    ../CommonConnectionPool.cpp
     ../friend.pb.cc
    ../usersqlite.cpp
    ../mysqlmodel.cpp
    ../localmessage.pb.cc
    ../user.pb.cc
    ../public.pb.cc
    ../redis.cpp
    ../group.pb.cc
    ../chatservice.pb.cc
)

# user 服务
set(SRC_LIST1
    ${COMMON_SRC}
    userservice.cc
    
)

# friend 服务（多一个 friend.pb.cc）
set(SRC_LIST2
    friendservice.cc
    ../friend.pb.cc
    ${COMMON_SRC}
)

# chat 服务（多一个 chatservice.pb.cc 和 chatserver.cc）
set(SRC_LIST3
    chatservice.cpp
    ../chatserver.cc
    ../friend.pb.cc
    ../chatservice.pb.cc
    ${COMMON_SRC}
)

set(SRC_LIST4
    groupservice.cc
    ../chatserver.cc
    ../friend.pb.cc
    ${COMMON_SRC}
)

# 三个可执行文件
add_executable(rpcuser ${SRC_LIST1})
add_executable(rpcfriend ${SRC_LIST2})
add_executable(rpcchat ${SRC_LIST3})
add_executable(rpcgroup ${SRC_LIST4})

# 链接所需库
target_link_libraries(rpcuser mprpc protobuf mysqlclient sqlite3 hiredis)
target_link_libraries(rpcfriend mprpc protobuf mysqlclient sqlite3 hiredis)
target_link_libraries(rpcchat mprpc protobuf mysqlclient sqlite3 hiredis)
target_link_libraries(rpcgroup mprpc protobuf mysqlclient sqlite3 hiredis)
