if (ENABLE_LINT)
    find_program(CLANG-TIDY_PATH clang-tidy)
    if(NOT CLANG-TIDY_PATH)
        message(WARNING "clang-tidy not found")
    endif()
else()
    message(STATUS "Clang-Tidy: Disabled")
endif()

function(AddClangTidy target)
    if(ENABLE_LINT AND CLANG-TIDY_PATH)
        set_target_properties("${target}"
            PROPERTIES
            CXX_CLANG_TIDY "${CLANG-TIDY_PATH}"
        )
    endif()
endfunction()
