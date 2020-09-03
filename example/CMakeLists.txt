add_executable(basic_example src/basic_example.cpp)

target_include_directories(basic_example PRIVATE include)
target_link_libraries(basic_example glShow2d)
