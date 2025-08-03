include(FetchContent)
FetchContent_Declare(
  googletest
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/deps/googletest/googletest-src
)
# For Windows: Prevent overriding the parent project's
# compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
option(INSTALL_GMOCK "Install GMock" OFF)
option(INSTALL_GTEST "Install GTest" OFF)
FetchContent_MakeAvailable(googletest)
