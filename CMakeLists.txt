cmake_minimum_required(VERSION 3.28.3)
project(PasswordManager VERSION 1.0.0 LANGUAGES C CXX)

option(ENABLE_COVERAGE "Enable Coverage" OFF)
option(ENABLE_LINT "Enable Lint" OFF)
option(ENABLE_ASAN "Enable AddressSanitizer (ASan)" OFF)
option(ENABLE_VALGRIND "Enable Valgrind" OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(NoInSourceBuilds)
include(ClangTidy)
include(Config)
include(Coverage)
include(CppCheck)
include(CTest)
include(Instrumentation)
include(Rust)
include(Testing)

add_subdirectory(crypto)
add_subdirectory(core)
add_subdirectory(tests)
