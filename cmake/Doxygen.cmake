if(NOT ENABLE_DOXYGEN)
    message(STATUS "Doxygen is disabled")
else()
    find_package(Doxygen)
    find_program(DOXYGEN_DOT_EXECUTABLE NAMES dot)

    if(DOXYGEN_FOUND)
        set(DOXYGEN_PROJECT_NAME "${PROJECT_NAME}")
        set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/docs")
        set(DOXYGEN_GENERATE_HTML YES)
    
        if(DOXYGEN_DOT_FOUND)
            set(DOXYGEN_HAVE_DOT YES)
            set(DOXYGEN_CALL_GRAPH YES)
            set(DOXYGEN_CALLER_GRAPH YES)
            set(DOXYGEN_CLASS_GRAPH YES)
            set(DOXYGEN_GRAPHICAL_HIERARCHY YES)
            set(DOXYGEN_DIRECTORY_GRAPH YES)
            set(DOXYGEN_DOT_IMAGE_FORMAT "svg")
            set(DOXYGEN_INTERACTIVE_SVG YES)
        else()
            message(WARNING "Doxygen Graphviz not found")
            set(DOXYGEN_HAVE_DOT NO)
        endif()

        doxygen_add_docs(docs
            "${PROJECT_SOURCE_DIR}/README.md"
            "${PROJECT_SOURCE_DIR}/core"
            ALL
            COMMENT "Generating documentation (Doxygen)..."
        )
    else()
        message(WARNING "Doxygen not found")
    endif()
endif()
