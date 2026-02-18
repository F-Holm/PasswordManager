if(ENABLE_VALGRIND)
    message(STATUS "Valgrind is enabled")
    find_program(VALGRIND_PATH valgrind)
    if(NOT VALGRIND_PATH)
        message(WARNING "valgrind not found")
        set(ENABLE_VALGRIND FALSE)
    endif()
else()
    message(STATUS "Valgrind is disabled")
endif()

function(AddValgrindInstrumentation target)
    if(ENABLE_VALGRIND)
        add_test(
            NAME "memcheck_${target}"
            COMMAND "${VALGRIND_PATH}" 
                    --leak-check=full 
                    --show-leak-kinds=all 
                    --track-origins=yes 
                    --error-exitcode=1 
                    "$<TARGET_FILE:${target}>"
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        )
    endif()
endfunction()
