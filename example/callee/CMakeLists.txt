# 通用的公共源文件（用于三个服务）



set(SRC_LIST1
    
    userservice.cc
    
)


set(SRC_LIST2
    friendservice.cc
    
    
)


set(SRC_LIST3
    chatservice.cpp
  
)

set(SRC_LIST4
    groupservice.cc
)

# 三个可执行文件
add_executable(rpcuser ${SRC_LIST1})
add_executable(rpcfriend ${SRC_LIST2})
add_executable(rpcchat ${SRC_LIST3})
add_executable(rpcgroup ${SRC_LIST4})

# 链接所需库
target_link_libraries(rpcuser mprpc tools)
target_link_libraries(rpcfriend mprpc tools)
target_link_libraries(rpcchat mprpc tools)
target_link_libraries(rpcgroup mprpc tools)
