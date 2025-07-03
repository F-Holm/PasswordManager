cmake_minimum_required(VERSION 3.28.3)

# Find compilers
find_program(CLANGXX clang++)
find_program(CLANG clang)
find_program(GXX g++)
find_program(GCC g)

# Set compilers
if(CLANGXX AND CLANG)
    set(CMAKE_CXX_COMPILER ${CLANGXX} CACHE STRING "C++ compiler" FORCE)
    set(CMAKE_C_COMPILER ${CLANG} CACHE STRING "C compiler" FORCE)
elseif(GXX AND GCC)
    message(WARNING "clang++ no encontrado, usando g++ en su lugar: ${GXX}")
    set(CMAKE_CXX_COMPILER ${GXX} CACHE STRING "C++ compiler" FORCE)
    set(CMAKE_C_COMPILER ${GCC} CACHE STRING "C compiler" FORCE)
else()
    message(FATAL_ERROR "No se encontró clang++ ni g++. Instala uno de los dos para continuar. Preferiblemente clang++")
endif()

# Project
project(GestorContra LANGUAGES CXX C)

# Configuration
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#message(CMAKE_CXX_FLAGS_DEBUG)
#message(CMAKE_CXX_FLAGS_RELEASE)
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -fno-inline")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Path y módulos
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(NoInSourceBuilds)
include(CTest)
include(OpenSSL)

add_subdirectory(src bin)
add_subdirectory(test)

# include(Packaging)