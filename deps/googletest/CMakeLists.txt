set(GOOGLETEST_SOURCE_DIR "${CMAKE_SOURCE_DIR}/deps/googletest/googletest-src")

if(NOT IS_DIRECTORY "${GOOGLETEST_SOURCE_DIR}" OR NOT EXISTS "${GOOGLETEST_SOURCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "GoogleTest no está en clonado"
    "\nDescargá los submódulos de git"
    "\nLo podés hacer mediante 'git submodule update --init --recursive'"
    "\nTambién podés usar los scripts de 'update_modules.sh' o 'reset_modules.sh'")
endif()

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
