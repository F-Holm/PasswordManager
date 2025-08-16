cmake_minimum_required(VERSION 3.28.3)
project(GestorContra VERSION 1.0.0 LANGUAGES C CXX)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    set(CMAKE_SYSROOT /opt/llvm-mingw/llvm-mingw/x86_64-w64-mingw32)
    set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -static")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(NoInSourceBuilds)
include(CTest)
include(Format)
include(Coverage)

add_subdirectory(deps)
add_subdirectory(src bin)
add_subdirectory(test)