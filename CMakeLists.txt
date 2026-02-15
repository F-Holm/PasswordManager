cmake_minimum_required(VERSION 3.28.3)
project(PasswordManager VERSION 1.0.0 LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(NoInSourceBuilds)
include(ClangTidy)
include(CppCheck)
include(CTest)
include(Format)
include(Rust)
include(Testing)

add_subdirectory(crypto)
add_subdirectory(core)
add_subdirectory(tests)
