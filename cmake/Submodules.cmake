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
