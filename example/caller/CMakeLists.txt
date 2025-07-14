
add_executable(consumer1
    main.cpp
)
add_executable(consumer2
    main2.cpp
)

target_link_libraries(consumer1 mprpc tools)
target_link_libraries(consumer2 mprpc tools)

