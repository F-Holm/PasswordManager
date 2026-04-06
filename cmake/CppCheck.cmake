if (ENABLE_LINT)
    if (WIN32)
        find_program(CPPCHECK_PATH
            NAMES cppcheck
            PATHS
            "C:/Program Files/Cppcheck"
            NO_DEFAULT_PATH
        )
    else()
        find_program(CPPCHECK_PATH cppcheck)
    endif()

    if(NOT CPPCHECK_PATH)
        message(WARNING "cppcheck not found")
    endif()
else()
    message(STATUS "CppCheck: Disabled")
endif()

function(AddCppCheck target)
    if (ENABLE_LINT AND CPPCHECK_PATH)
        set_target_properties("${target}"
            PROPERTIES CXX_CPPCHECK
            "${CPPCHECK_PATH};--enable=warning;--error-exitcode=1;--force;--quiet"
        )
    endif()
endfunction()
