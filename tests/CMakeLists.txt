set(This
    ConcurrentListTests)

set(Sources 
    src/ConcurrentListTests.cpp)

add_executable(${This} ${Sources})

target_link_libraries(${This} 
    ConcurrentList gtest_main pthread)

add_test(
    NAME ${This}
    COMMAND ${This}
)