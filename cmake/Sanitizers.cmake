include_guard(GLOBAL)

# Runtime sanitizers (Clang/GCC, Debug builds only).
#
# Compatibility:
#   - ASan, LSan and UBSan can be combined (LSan is already part of ASan).
#   - TSan cannot be combined with ASan or LSan, so it needs its own build.
#   - UBSan can be combined with any of them.
#   - MSan is not supported yet.
#
# SANITIZER_TEST_ENV holds the runtime options passed to every test.

set(SANITIZER_TEST_ENV "")

function(_SanitizersValidate)
    if(ENABLE_TSAN AND (ENABLE_ASAN OR ENABLE_LSAN))
        message(FATAL_ERROR
            "Sanitizers: TSan cannot be combined with ASan or LSan. "
            "Use a separate build directory (e.g. `just sanitize tsan`).")
    endif()
    if(ENABLE_TSAN AND WIN32)
        message(FATAL_ERROR "Sanitizers: TSan is not supported on Windows")
    endif()
endfunction()

macro(AddSanitizers)
    _SanitizersValidate()

    if(WIN32)
        if(ENABLE_ASAN OR ENABLE_LSAN OR ENABLE_UBSAN)
            message(STATUS "Sanitizers: Disabled on Windows")
        endif()
    else()
        set(_sanitizers "")
        set(_test_env "")

        # Apple's toolchain does not ship LeakSanitizer (neither standalone
        # nor as ASan's detect_leaks), so LSan is skipped there.
        set(_lsan "${ENABLE_LSAN}")
        if(_lsan AND APPLE)
            message(WARNING "Sanitizers: LSan is not supported on macOS, skipping it")
            set(_lsan OFF)
        endif()

        if(ENABLE_ASAN)
            list(APPEND _sanitizers address)
            add_compile_options(-fsanitize-address-use-after-scope)
            set(_asan_options
                "abort_on_error=1:strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1")
            if(_lsan)
                string(PREPEND _asan_options "detect_leaks=1:")
            endif()
            list(APPEND _test_env "ASAN_OPTIONS=${_asan_options}")
        elseif(_lsan)
            list(APPEND _sanitizers leak)
        endif()

        if(_lsan)
            list(APPEND _test_env "LSAN_OPTIONS=report_objects=1")
        endif()

        if(ENABLE_UBSAN)
            list(APPEND _sanitizers undefined)
            if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
                list(APPEND _sanitizers integer nullability)
                add_compile_options(
                    "-fsanitize-ignorelist=${CMAKE_SOURCE_DIR}/cmake/ubsan_ignorelist.txt")
            endif()
            add_compile_options(-fno-sanitize-recover=all)
            list(APPEND _test_env "UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1")
        endif()

        if(ENABLE_TSAN)
            list(APPEND _sanitizers thread)
            list(APPEND _test_env
                "TSAN_OPTIONS=halt_on_error=1:second_deadlock_stack=1:suppressions=${CMAKE_SOURCE_DIR}/cmake/tsan_suppressions.txt")
        endif()

        if(_sanitizers)
            list(JOIN _sanitizers "," _sanitizer_list)
            message(STATUS "Sanitizers: -fsanitize=${_sanitizer_list}")
            add_compile_options("-fsanitize=${_sanitizer_list}" -fno-omit-frame-pointer)
            add_link_options("-fsanitize=${_sanitizer_list}")
            set(SANITIZER_TEST_ENV "${_test_env}")
        endif()
    endif()
endmacro()
