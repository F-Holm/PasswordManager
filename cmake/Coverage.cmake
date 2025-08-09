include(Platform)

function(AddCoverage target)
    require_linux_x64(Coverage)
    if (${CMAKE_C_COMPILER} STREQUAL "clang" AND ${CMAKE_CXX_COMPILER} STREQUAL "clang++")
        AddCoverageClang(${target})
    elseif (${CMAKE_C_COMPILER} STREQUAL "gcc" AND ${CMAKE_CXX_COMPILER} STREQUAL "g++")
        AddCoverageGcc(${target})
    endif()
endfunction()

function(CleanCoverage target)
    require_linux_x64(Coverage)
    if (${CMAKE_C_COMPILER} STREQUAL "clang" AND ${CMAKE_CXX_COMPILER} STREQUAL "clang++")
        CleanCoverageClang(${target})
    elseif (${CMAKE_C_COMPILER} STREQUAL "gcc" AND ${CMAKE_CXX_COMPILER} STREQUAL "g++")
        CleanCoverageGcc(${target})
    endif()
endfunction()

function(InstrumentForCoverage target)
    require_linux_x64(Coverage)
    if (${CMAKE_C_COMPILER} STREQUAL "clang" AND ${CMAKE_CXX_COMPILER} STREQUAL "clang++")
        InstrumentForCoverageClang(${target})
    elseif (${CMAKE_C_COMPILER} STREQUAL "gcc" AND ${CMAKE_CXX_COMPILER} STREQUAL "g++")
        InstrumentForCoverageGcc(${target})
    endif()
endfunction()

# Clang
function(AddCoverageClang target)
    find_program(LLVM_PROFDATA llvm-profdata REQUIRED)
    find_program(LLVM_COV llvm-cov REQUIRED)

    add_custom_target(coverage-${target}
        # Limpia datos previos de profiling
        COMMAND ${LLVM_PROFDATA} merge -sparse default.profraw -o default.profdata || true

        # Ejecuta el target para generar archivos .profraw
        COMMAND $<TARGET_FILE:${target}>

        # Genera el reporte HTML con llvm-cov
        COMMAND ${LLVM_COV} show
                $<TARGET_FILE:${target}>
                -instr-profile=default.profdata
                -format=html
                -output-dir=coverage-${target}
                -ignore-filename-regex='.*(\/usr\/include\/|tests?).*'

        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endfunction()

function(CleanCoverageClang target)
    add_custom_command(TARGET ${target} PRE_BUILD COMMAND
        cmake -E rm -f ${CMAKE_BINARY_DIR}/default.profraw
        cmake -E rm -rf ${CMAKE_BINARY_DIR}/coverage-${target}
    )
endfunction()

function(InstrumentForCoverageClang target)
    if (CMAKE_BUILD_TYPE STREQUAL Debug)
        target_compile_options(${target} PRIVATE -fprofile-instr-generate -fcoverage-mapping)
        target_link_options(${target} PRIVATE -fprofile-instr-generate -fcoverage-mapping)
    endif()
endfunction()

# GCC
function(AddCoverageGcc target)
    find_program(LCOV_PATH lcov REQUIRED)
    find_program(GENHTML_PATH genhtml REQUIRED)
    add_custom_target(coverage-${target}
        COMMAND ${LCOV_PATH} -d . --zerocounters
        COMMAND $<TARGET_FILE:${target}>
        COMMAND ${LCOV_PATH} -d . --capture -o coverage.info
        COMMAND ${LCOV_PATH} -r coverage.info '/usr/include/*'
                             -o filtered.info
        COMMAND ${GENHTML_PATH} -o coverage-${target}
                                filtered.info --legend
        COMMAND rm -rf coverage.info filtered.info
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endfunction()

function(CleanCoverageGcc target)
    add_custom_command(TARGET ${target} PRE_BUILD COMMAND
                       find ${CMAKE_BINARY_DIR} -type f
                       -name '*.gcda' -exec cmake -E rm {} +)
endfunction()

function(InstrumentForCoverageGcc target)
    if (CMAKE_BUILD_TYPE STREQUAL Debug)
        target_compile_options(${target} 
                               PRIVATE --coverage -fno-inline)
        target_link_options(${target} PUBLIC --coverage)
    endif()
endfunction()
