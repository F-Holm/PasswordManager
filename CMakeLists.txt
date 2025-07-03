cmake_minimum_required(VERSION 3.28.3)
project(GestorContra VERSION 1.0.0 LANGUAGES CXX C)

# Configuration
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(CMAKE_CXX_FLAGS_DEBUG)
message(CMAKE_CXX_FLAGS_RELEASE)
#set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -fno-inline")
#set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Path y módulos
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(NoInSourceBuilds)

add_subdirectory(src bin)
#add_subdirectory(test)

# include(Packaging)