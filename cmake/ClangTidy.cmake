find_program(CLANG-TIDY_PATH clang-tidy)
if(NOT CLANG-TIDY_PATH)
    message(WARNING "clang-tidy not found")
endif()

function(AddClangTidy target)
    if(CLANG-TIDY_PATH)
        set_target_properties("${target}"
            PROPERTIES
            CXX_CLANG_TIDY "${CLANG-TIDY_PATH}"
        )
    endif()
endfunction()
