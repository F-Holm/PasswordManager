include_guard(GLOBAL)

include(Sanitizers)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    AddSanitizers()
    if(NOT SANITIZER_DEFAULT_OPTIONS_SOURCE)
        message(STATUS "No dynamic instrumentation enabled")
    endif()
endif()

function(AddInstrumentation target)
endfunction()
