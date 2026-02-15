include(FetchContent)

FetchContent_Declare(
  corrosion
  GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
  GIT_TAG v0.6.1
)

FetchContent_MakeAvailable(corrosion)

function(enable_rust_clippy TARGET_NAME WORKING_DIR)
    find_program(CARGO_CLIPPY cargo-clippy)
    if(NOT CARGO_CLIPPY)
        message(WARNING "cargo-clippy not found. Skipping clippy enforcement for ${TARGET_NAME}.")
        return()
    endif()

    add_custom_target(${TARGET_NAME}_clippy_check
        COMMAND cargo clippy --all-targets --all-features -- -D warnings
        WORKING_DIRECTORY ${WORKING_DIR}
        COMMENT "Running cargo clippy (strict) for ${TARGET_NAME}"
        VERBATIM
    )
    add_dependencies(${TARGET_NAME} ${TARGET_NAME}_clippy_check)
endfunction()

function(enable_rust_fmt TARGET_NAME WORKING_DIR)
    find_program(CARGO_FMT cargo-fmt)
    if(NOT CARGO_FMT)
        message(WARNING "cargo-fmt not found. Skipping rust fmt for ${TARGET_NAME}.")
        return()
    endif()

    add_custom_target(${TARGET_NAME}_fmt
        COMMAND cargo fmt --all
        WORKING_DIRECTORY ${WORKING_DIR}
        COMMENT "Running cargo fmt for ${TARGET_NAME}"
        VERBATIM
    )
    add_dependencies(${TARGET_NAME} ${TARGET_NAME}_fmt)
endfunction()
