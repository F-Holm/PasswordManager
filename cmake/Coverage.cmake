set(COVERAGE_SUPPORTED FALSE)

if(NOT ENABLE_COVERAGE)
    message(STATUS "Coverage is disabled")
elseif(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Coverage disabled in Release")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "GNU")
    find_program(LCOV_PATH lcov)
    find_program(GENHTML_PATH genhtml)
    
    if(LCOV_PATH AND GENHTML_PATH)
        set(COVERAGE_SUPPORTED TRUE)
        set(COVERAGE_HANDLER "GCC")
        message(STATUS "[Coverage] GCC detected. Using lcov/genhtml engine.")
    else()
        message(WARNING "[Coverage] GCC detected but lcov or genhtml tools were not found. Coverage targets will be skipped.")
    endif()

elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    find_program(LLVM_PROFDATA_PATH llvm-profdata)
    find_program(LLVM_COV_PATH llvm-cov)
    find_program(GENHTML_PATH genhtml)

    if(LLVM_PROFDATA_PATH AND LLVM_COV_PATH AND GENHTML_PATH)
        set(COVERAGE_SUPPORTED TRUE)
        set(COVERAGE_HANDLER "CLANG")
        message(STATUS "[Coverage] Clang detected. Using llvm-cov/genhtml engine.")
    else()
        message(WARNING "[Coverage] Clang detected but LLVM coverage tools (llvm-profdata/llvm-cov) were not found.")
    endif()

else()
    message(WARNING "[Coverage] Compiler ${CMAKE_CXX_COMPILER_ID} is not supported for automated coverage reporting.")
endif()


function(InstrumentForCoverage target)
    if(NOT COVERAGE_SUPPORTED)
        return()
    endif()

    get_target_property(target_type ${target} TYPE)
    set(SCOPE_1 PRIVATE)
    set(SCOPE_2 PUBLIC)
    if(target_type STREQUAL "INTERFACE_LIBRARY")
        set(SCOPE_1 INTERFACE)
        set(SCOPE_2 INTERFACE)
    endif()

    if(COVERAGE_HANDLER STREQUAL "GCC")
        target_compile_options(${target} ${SCOPE_1} --coverage -fno-inline)
        target_link_options(${target} ${SCOPE_2} --coverage)
    else()
        target_compile_options(${target} ${SCOPE_1} -fprofile-instr-generate -fcoverage-mapping)
        target_link_options(${target} ${SCOPE_2} -fprofile-instr-generate -fcoverage-mapping)
    endif()
endfunction()


function(AddCoverage target)
    if(NOT COVERAGE_SUPPORTED)
        return()
    endif()

    set(COVERAGE_FILE "${target}_coverage.info")
    set(FILTERED_FILE "${target}_filtered.info")

    if(COVERAGE_HANDLER STREQUAL "GCC")
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${LCOV_PATH} -d . --zerocounters
            COMMAND $<TARGET_FILE:${target}>
            COMMAND ${LCOV_PATH} -d . --capture -o ${COVERAGE_FILE}
            COMMAND ${LCOV_PATH} -r ${COVERAGE_FILE} '/usr/include/*' '*/tests/*' '*/test/*' -o ${FILTERED_FILE}
            COMMAND ${GENHTML_PATH} -o coverage-${target} ${FILTERED_FILE} --legend
            COMMAND ${CMAKE_COMMAND} -E rm -f ${COVERAGE_FILE} ${FILTERED_FILE}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating HTML coverage report for ${target} (GCC/LCOV)"
        )

    else()
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE="${target}.profraw" $<TARGET_FILE:${target}>
            COMMAND ${LLVM_PROFDATA_PATH} merge -sparse ${target}.profraw -o ${target}.profdata
            COMMAND ${LLVM_COV_PATH} export $<TARGET_FILE:${target}> -instr-profile=${target}.profdata -format=lcov > ${COVERAGE_FILE}
            COMMAND ${GENHTML_PATH} -o coverage-${target} ${COVERAGE_FILE} --legend
            COMMAND ${CMAKE_COMMAND} -E rm -f ${target}.profraw ${target}.profdata ${COVERAGE_FILE}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating HTML coverage report for ${target} (Clang/LLVM)"
        )
    endif()
endfunction()


function(CleanCoverage target)
    if(NOT COVERAGE_SUPPORTED)
        return()
    endif()

    if(COVERAGE_HANDLER STREQUAL "GCC")
        add_custom_command(TARGET ${target} PRE_BUILD
            COMMAND ${LCOV_PATH} -d . --zerocounters || ${CMAKE_COMMAND} -E true
            COMMAND ${CMAKE_COMMAND} -E rm -f "${target}_coverage.info" "${target}_filtered.info"
            COMMAND ${CMAKE_COMMAND} -E rm -rf "coverage-${target}"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Cleaning up GCC coverage counters."
        )
    else()
        add_custom_command(TARGET ${target} PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E rm -f "${target}.profraw" "${target}.profdata" "${target}_coverage.info"
            COMMAND ${CMAKE_COMMAND} -E rm -rf "coverage-${target}"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Cleaning up Clang coverage files."
        )
    endif()
endfunction()
