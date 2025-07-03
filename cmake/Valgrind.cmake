function(AddValgrind target)
  if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(STATUS "Valgrind no se configura porque no estás en Linux.")
    return()
  endif()

  find_program(VALGRIND_PATH valgrind REQUIRED)
  add_custom_target(valgrind
    COMMAND ${VALGRIND_PATH} --leak-check=yes
            $<TARGET_FILE:${target}>
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )
endfunction()
