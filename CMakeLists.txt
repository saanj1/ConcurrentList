set(This concurrent_list)
add_library(${This} INTERFACE)
target_include_directories(${This} INTERFACE include/)
add_subdirectory(tests)