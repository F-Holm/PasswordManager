include_guard(GLOBAL)

include(ASan)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(ENABLE_ASAN)
        AddASanInstrumentation()
    else()
        message(STATUS "No dynamic instrumentation enabled")
    endif()
endif()

function(AddInstrumentation target)
endfunction()
