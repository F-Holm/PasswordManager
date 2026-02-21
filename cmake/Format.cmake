find_program(CLANG-FORMAT_PATH clang-format)
if(NOT CLANG-FORMAT_PATH)
    message(WARNING "clang-format not found")
endif()

set(FORMAT_ID 0)

function(Format target directory)
    if(CLANG-FORMAT_PATH)
        set(EXPRESSION h hpp hh c cc cxx cpp)
        list(TRANSFORM EXPRESSION PREPEND "${directory}/*.")
        file(GLOB_RECURSE SOURCE_FILES FOLLOW_SYMLINKS
            LIST_DIRECTORIES false ${EXPRESSION}
        )

        math(EXPR LOCAL_COUNT "${FORMAT_ID} + 1")
        set(FORMAT_ID ${LOCAL_COUNT} PARENT_SCOPE)

        set(FORMAT_STEP_NAME "format_${target}_${FORMAT_ID}")
        add_custom_target(${FORMAT_STEP_NAME}
            COMMAND "${CLANG-FORMAT_PATH}" -i --style=file ${SOURCE_FILES}
            COMMENT "Formateando código en ${directory} para el target ${target}..."
            VERBATIM
        )
        add_dependencies(${target} ${FORMAT_STEP_NAME})
    endif()
endfunction()

function(FormatTest target)
    Format(${target} ".")
endfunction()

function(FormatModule target)
    Format(${target} "./include/*")
    get_target_property(target_type ${target} TYPE)
    if(NOT target_type STREQUAL "INTERFACE_LIBRARY")
        Format(${target} ".")
    endif()
endfunction()
