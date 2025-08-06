set(MEMCHECK_COVER_SOURCE_DIR "${CMAKE_SOURCE_DIR}/deps/memcheck-cover/memcheck-cover-src")

verify_submodule_cloned("memcheck-cover" "${MEMCHECK_COVER_SOURCE_DIR}" "README.md")

include(FetchContent)
FetchContent_Declare(
    memcheck-cover
    SOURCE_DIR "${MEMCHECK_COVER_SOURCE_DIR}"
)
FetchContent_MakeAvailable(memcheck-cover)