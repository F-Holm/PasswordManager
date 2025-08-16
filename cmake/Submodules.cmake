set(CHECK_FILE "proceso-exitosa")

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
    if(IS_DIRECTORY "${BUIL0D_DIR}")
        file(WRITE "${BUILD_DIR}/${CHECK_FILE}")
    endif()
endfunction()

function(CheckBuilt BUILD_DIR BUILT)
    if(IS_DIRECTORY "${BUIL0D_DIR}")
        if(EXISTS "${BUILD_DIR}/${CHECK_FILE}")
            set(BUILT TRUE)
            return()
        else()
            file(REMOVE_RECURSE "${BUILD_DIR}")
        endif()
    endif()
    set(BUILT FALSE)
endfunction()

function(AddCheckCopy SOURCE_DIR)
    if(IS_DIRECTORY "${SOURCE_DIR}")
        file(WRITE "${SOURCE_DIR}/${CHECK_FILE}")
    endif()
endfunction()

function(CheckCopy SOURCE_DIR COPY)
    if(IS_DIRECTORY "${SOURCE_DIR}")
        if(EXISTS "${SOURCE_DIR}/${CHECK_FILE}")
            set(COPY TRUE)
            return()
        else()
            file(REMOVE_RECURSE "${SOURCE_DIR}")
        endif()
    endif()
    set(COPY FALSE)
endfunction()