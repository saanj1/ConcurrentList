set(This
    concurrent_list_test)

set(Sources 
    src/concurrent_list_test.cpp)

add_executable(${This} ${Sources})

target_link_libraries(${This} concurrent_list pthread)
