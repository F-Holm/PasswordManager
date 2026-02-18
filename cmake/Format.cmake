find_program(CLANG-FORMAT_PATH clang-format)
if(NOT CLANG-FORMAT_PATH)
    message(WARNING "clang-format not found")
endif()

function(Format target directory)
    if(CLANG-FORMAT_PATH)
        set(EXPRESSION h hpp hh c cc cxx cpp)
        list(TRANSFORM EXPRESSION PREPEND "${directory}/*.")
        file(GLOB_RECURSE SOURCE_FILES FOLLOW_SYMLINKS
            LIST_DIRECTORIES false ${EXPRESSION}
        )
        add_custom_command(TARGET "${target}" PRE_BUILD COMMAND
            "${CLANG-FORMAT_PATH}" -i --style=file ${SOURCE_FILES}
        )
    endif()
endfunction()

function(FormatTest target)
    Format(${target} ".")
endfunction()

function(FormatModule target)
    Format(${target} ".")
    Format(${target} "./include/*")
endfunction()
