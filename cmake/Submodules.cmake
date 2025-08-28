set(CHECK_FILE "proceso-exitoso")

function(verify_submodule_cloned submodule_name submodule_dir required_file)
    if(NOT IS_DIRECTORY "${submodule_dir}" OR NOT EXISTS "${submodule_dir}/${required_file}")
        message(FATAL_ERROR
            "'${submodule_name}' no está clonado correctamente."
            "\n\n⚠️  Posiblemente falte inicializar el submódulo de Git."
            "\n📂 Ruta esperada: ${submodule_dir}"
            "\n📄 Archivo esperado: ${required_file}"
            "\n💡 Solución:"
            "\n  git submodule update --init --recursive"
            "\n  o bien ejecutá ./update_modules.sh o ./reset_modules.sh"
        )
    endif()
endfunction()

function(AddCheckBuilt BUILD_DIR)
    if(IS_DIRECTORY "${BUILD_DIR}")
        file(WRITE "${BUILD_DIR}/${CHECK_FILE}")
    endif()
endfunction()

function(CheckBuilt BUILD_DIR)
    if(IS_DIRECTORY "${BUILD_DIR}")
        if(EXISTS "${BUILD_DIR}/${CHECK_FILE}")
            message("${BUILD_DIR}/${CHECK_FILE} ENCONTRADO")
            set(BUILT TRUE PARENT_SCOPE)
            return()
            message(FATAL_ERROR  "ERROR RETURN")
        else()
            file(REMOVE_RECURSE "${BUILD_DIR}")
        endif()
    endif()
    message("${BUILD_DIR}/${CHECK_FILE} NO ENCONTRADO")
    set(BUILT FALSE PARENT_SCOPE)
endfunction()

function(AddCheckCopy SOURCE_DIR)
    if(IS_DIRECTORY "${SOURCE_DIR}")
        file(WRITE "${SOURCE_DIR}/${CHECK_FILE}")
    endif()
endfunction()

function(CheckCopy SOURCE_DIR)
    if(IS_DIRECTORY "${SOURCE_DIR}")
        if(EXISTS "${SOURCE_DIR}/${CHECK_FILE}")
            message("${SOURCE_DIR}/${CHECK_FILE} ENCONTRADO")
            set(COPY TRUE PARENT_SCOPE)
            return()
            message(FATAL_ERROR  "ERROR RETURN")
        else()
            file(REMOVE_RECURSE "${SOURCE_DIR}")
        endif()
    endif()
    message("${SOURCE_DIR}/${CHECK_FILE} NO ENCONTRADO")
    set(COPY FALSE PARENT_SCOPE)
endfunction()