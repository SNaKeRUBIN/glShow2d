cmake_minimum_required(VERSION 3.14)

project(glShow2d LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 17)

set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/ThirdParty/)
set(THIRD_PARTY_BUILD_DIR ${THIRD_PARTY_DIR}/build)
set(EXTERNAL_INSTALL_DIR ${THIRD_PARTY_BUILD_DIR}/install)

execute_process(
    COMMAND ${CMAKE_COMMAND} -B build -G "${CMAKE_GENERATOR}" -DCMAKE_INSTALL_PREFIX=install
    WORKING_DIRECTORY ${THIRD_PARTY_DIR}
)

execute_process(
    COMMAND ${CMAKE_COMMAND} --build . --config Release
    WORKING_DIRECTORY ${THIRD_PARTY_BUILD_DIR}
)

find_package(glad 
    REQUIRED
    PATHS ${EXTERNAL_INSTALL_DIR}
    NO_DEFAULT_PATH
)

find_package(freetype 
    REQUIRED
    PATHS ${EXTERNAL_INSTALL_DIR}
    NO_DEFAULT_PATH
)

find_package(glfw3
    REQUIRED
    PATHS ${EXTERNAL_INSTALL_DIR}
    NO_DEFAULT_PATH
)

add_library(glShow2d STATIC 
    glShow2d/src/glShow2d.cpp
)

set_target_properties(glShow2d 
    PROPERTIES 
        PUBLIC_HEADER ${CMAKE_SOURCE_DIR}/glShow2d/include/glShow2d.h
)

target_include_directories(glShow2d PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/glShow2d/include>
    $<INSTALL_INTERFACE:include/glShow2d>
)

target_link_libraries(glShow2d glad::glad freetype glfw)

set(glShow2dTarget glShow2dConfig)
install(TARGETS glShow2d
    EXPORT ${glShow2dTarget}
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include/glShow2d
)

install(EXPORT ${glShow2dTarget}
    FILE ${glShow2dTarget}.cmake
    DESTINATION lib/cmake/glShow2d
)

add_subdirectory(example)
