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
    # gtest_discover_tests() flattens PROPERTIES into a single list, so the
    # separators of the ENVIRONMENT list must be escaped to survive.
    string(REPLACE ";" "\\;" _test_env "${SANITIZER_TEST_ENV}")
    gtest_discover_tests("${target}"
        DISCOVERY_MODE PRE_TEST
        PROPERTIES ENVIRONMENT "${_test_env}")
endmacro()
