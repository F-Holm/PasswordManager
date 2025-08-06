set(MEMCHECK_COVER_SOURCE_DIR "${CMAKE_SOURCE_DIR}/deps/memcheck-cover/memcheck-cover-src")

if(NOT IS_DIRECTORY "${MEMCHECK_COVER_SOURCE_DIR}" OR NOT EXISTS "${MEMCHECK_COVER_SOURCE_DIR}/README.md")
    message(FATAL_ERROR "memcheck-cover no está en clonado"
    "\nDescargá los submódulos de git"
    "\nLo podés hacer mediante 'git submodule update --init --recursive'"
    "\nTambién podés usar los scripts de 'update_modules.sh' o 'reset_modules.sh'")
endif()

include(FetchContent)
FetchContent_Declare(
    memcheck-cover
    SOURCE_DIR "${MEMCHECK_COVER_SOURCE_DIR}"
)
FetchContent_MakeAvailable(memcheck-cover)