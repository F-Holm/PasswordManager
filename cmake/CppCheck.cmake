function(AddCppCheck target)
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

    if (CPPCHECK_PATH)
        set_target_properties("${target}"
            PROPERTIES CXX_CPPCHECK
            "${CPPCHECK_PATH};--enable=warning;--error-exitcode=1;--force;--quiet"
        )
    else()
        message(WARNING "cppcheck not found. Skipping cppcheck for ${target}.")
    endif()
endfunction()
