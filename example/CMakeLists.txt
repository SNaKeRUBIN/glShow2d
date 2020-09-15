add_executable(basic_example src/basic_example.cpp)

file(DOWNLOAD https://github.com/nothings/stb/blob/master/stb_image.h ${CMAKE_CURRENT_SOURCE_DIR}/include/stb_image.h)
target_include_directories(basic_example PRIVATE include)
target_link_libraries(basic_example glShow2d)
add_dependencies(basic_example glShow2dCombine)
