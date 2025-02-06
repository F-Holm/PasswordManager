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

# Find OpenSSL
find_package(OpenSSL REQUIRED)
if (NOT OPENSSL_FOUND)
  message(FATAL_ERROR "OpenSSL not found. Please install OpenSSL and try again.")
endif()

# Include OpenSSL headers
include_directories(${OPENSSL_INCLUDE_DIR})

# Add source to this project's executable.
add_executable (PasswordManager ${SOURCES})

# Link OpenSSL libraries
target_link_libraries(PasswordManager ${OPENSSL_LIBRARIES})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET PasswordManager PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
