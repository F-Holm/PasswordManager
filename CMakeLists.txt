# CMakeList.txt : CMake project for PasswordManager, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("PasswordManager")

# Include all headers in /include
include_directories(include)

# Collect all source files in /src
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Add source to this project's executable.
add_executable (PasswordManager ${SOURCES})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET PasswordManager PROPERTY CXX_STANDARD 20)
endif()

# --- Google Test ---

# --- Add Google Test ---
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG main  # O una versión específica como v1.14.0
  GIT_SHALLOW TRUE  # Solo descarga la última versión sin historial de Git
)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Collect all test files in /tests
file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")

# Create test executable
add_executable(PasswordManagerTests ${TEST_SOURCES} ${SOURCES})#tests/PasswordManagerTests.cpp)

# Link Google Test
target_link_libraries(PasswordManagerTests gtest gtest_main)

# Add tests to CTest
include(GoogleTest)
gtest_discover_tests(PasswordManagerTests)

# --- OpenSSL ---

# Find OpenSSL
find_package(OpenSSL REQUIRED)
if (NOT OPENSSL_FOUND)
  message(FATAL_ERROR "OpenSSL not found. Please install OpenSSL and try again.")
endif()

# Include OpenSSL headers
include_directories(${OPENSSL_INCLUDE_DIR})

# Link OpenSSL libraries
target_link_libraries(PasswordManager ${OPENSSL_LIBRARIES})
target_link_libraries(PasswordManagerTests ${OPENSSL_LIBRARIES})

# TODO: Add tests and install targets if needed.
