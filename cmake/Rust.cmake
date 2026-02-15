include(FetchContent)

FetchContent_Declare(
  corrosion
  GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
  GIT_TAG v0.6.1
)

FetchContent_MakeAvailable(corrosion)

find_program(CARGO_EXECUTABLE cargo REQUIRED)
find_program(CARGO_FMT cargo-fmt)
find_program(CARGO_CLIPPY cargo-clippy)

if(NOT CARGO_FMT)
    message(WARNING "cargo-fmt not found")
endif()
if(NOT CARGO_CLIPPY)
    message(WARNING "cargo-clippy not found")
endif()

function(enable_rust_fmt TARGET_NAME WORKING_DIR)
    if(CARGO_FMT)
        add_custom_target(${TARGET_NAME}_fmt
            COMMAND ${CARGO_EXECUTABLE} fmt --all
            WORKING_DIRECTORY ${WORKING_DIR}
            COMMENT "Running cargo fmt for ${TARGET_NAME}"
            VERBATIM
        )
        add_dependencies(${TARGET_NAME} ${TARGET_NAME}_fmt)
    endif()
endfunction()

function(enable_rust_clippy TARGET_NAME WORKING_DIR)
    if(CARGO_CLIPPY)
        add_custom_target(${TARGET_NAME}_clippy_check
            COMMAND ${CARGO_EXECUTABLE} clippy --all-targets --all-features -- -D warnings
            WORKING_DIRECTORY ${WORKING_DIR}
            COMMENT "Running cargo clippy (strict) for ${TARGET_NAME}"
            VERBATIM
        )
        add_dependencies(${TARGET_NAME} ${TARGET_NAME}_clippy_check)
    endif()
endfunction()

function(enable_rust_tests TARGET_NAME WORKING_DIR)
    add_test(
        NAME ${TARGET_NAME}_tests
        COMMAND ${CARGO_EXECUTABLE} test --manifest-path ${WORKING_DIR}/Cargo.toml
        WORKING_DIRECTORY ${WORKING_DIR}
    )
endfunction()
