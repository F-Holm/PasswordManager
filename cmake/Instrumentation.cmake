include_guard(GLOBAL)

include(Sanitizers)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    AddSanitizers()
    if(NOT SANITIZER_TEST_ENV)
        message(STATUS "No dynamic instrumentation enabled")
    endif()
endif()

function(AddInstrumentation target)
endfunction()
