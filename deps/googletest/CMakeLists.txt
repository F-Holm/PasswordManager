set(GOOGLETEST_SOURCE_DIR "${CMAKE_SOURCE_DIR}/deps/googletest/googletest-src")

verify_submodule_cloned("GoogleTest" "${GOOGLETEST_SOURCE_DIR}" "CMakeLists.txt")

include(FetchContent)
FetchContent_Declare(
  googletest
  SOURCE_DIR "${GOOGLETEST_SOURCE_DIR}"
)
# For Windows: Prevent overriding the parent project's
# compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
option(INSTALL_GMOCK "Install GMock" OFF)
option(INSTALL_GTEST "Install GTest" OFF)
FetchContent_MakeAvailable(googletest)
