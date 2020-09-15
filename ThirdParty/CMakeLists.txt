cmake_minimum_required(VERSION 3.14)

project(OpenGL_lib LANGUAGES NONE)
set(CMAKE_CXX_STANDARD 17)

include(ExternalProject)

ExternalProject_Add(Glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG 32b58e5
    INSTALL_DIR install
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DGLAD_PROFILE:STRING="core"
        -DGLAD_API:STRING="gl=4.6"
        -DGLAD_INSTALL=ON
)

ExternalProject_Add(GLFW
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3
    INSTALL_DIR install
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
        -DGLFW_BUILD_EXAMPLES=OFF
        -DGLFW_BUILD_TESTS=OFF
        -DGLFW_BUILD_DOCS=OFF
        -DGLFW_BUILD_EXAMPLES=OFF
)

ExternalProject_Add(freetype2
    URL https://download.savannah.gnu.org/releases/freetype/freetype-2.10.1.tar.gz
    INSTALL_DIR install
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DCMAKE_DISABLE_FIND_PACKAGE_HarfBuzz=TRUE
        -DCMAKE_DISABLE_FIND_PACKAGE_PNG=TRUE
        -DCMAKE_DISABLE_FIND_PACKAGE_ZLIB=TRUE
        -DCMAKE_DISABLE_FIND_PACKAGE_BZip2=TRUE
)
