cmake_minimum_required(VERSION 3.28.3)
project(GestorContra VERSION 1.0.0 LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(NoInSourceBuilds)
include(CTest)
include(Format)
include(Coverage)
include(OpenSSL)

add_subdirectory(deps)
add_subdirectory(src bin)
add_subdirectory(test)