include_guard(GLOBAL)

include(Dependencies)
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY ${GOOGLETEST_GIT_REPOSITORY}
    GIT_TAG ${GOOGLETEST_GIT_TAG}
    SYSTEM
)
# For Windows: Prevent overriding the parent project's
# compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
option(INSTALL_GMOCK "Install GMock" OFF)
option(INSTALL_GTEST "Install GTest" OFF)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

macro(AddTests target)
    target_link_libraries("${target}" PRIVATE gtest_main gmock)
    # Sanitizer runtime options (see cmake/Sanitizers.cmake).
    if(SANITIZER_DEFAULT_OPTIONS_SOURCE)
        target_sources("${target}" PRIVATE "${SANITIZER_DEFAULT_OPTIONS_SOURCE}")
    endif()
    gtest_discover_tests("${target}" DISCOVERY_MODE PRE_TEST)
endmacro()
