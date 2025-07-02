cmake_minimum_required(VERSION 4.0.2)
project(GestorContra LANGUAGES CXX)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(NoInSourceBuilds)

include(CTest)

add_subdirectory(src bin)
add_subdirectory(test)

include(Packaging)