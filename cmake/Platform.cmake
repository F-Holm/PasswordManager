macro(require_linux_x64 target_name)
    if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" OR 
       NOT ${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64|AMD64")
        message(STATUS "${target_name} no se va a configurar porque no estás en Linux x64")
        return()
    endif()
    message(STATUS "Configurando ${target_name} para Linux x64")
endmacro()