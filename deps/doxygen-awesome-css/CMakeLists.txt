set(DOXYGEN_AWESOME_CSS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/deps/doxygen-awesome-css/doxygen-awesome-css-src")

if(NOT IS_DIRECTORY "${DOXYGEN_AWESOME_CSS_SOURCE_DIR}" OR NOT EXISTS "${DOXYGEN_AWESOME_CSS_SOURCE_DIR}/README.md")
    message(FATAL_ERROR "doxygen-awesome-css no está en clonado"
    "\nDescargá los submódulos de git"
    "\nLo podés hacer mediante 'git submodule update --init --recursive'"
    "\nTambién podés usar los scripts de 'update_modules.sh' o 'reset_modules.sh'")
endif()

include(FetchContent)
FetchContent_Declare(
  doxygen-awesome-css
  SOURCE_DIR "${DOXYGEN_AWESOME_CSS_SOURCE_DIR}"
)
FetchContent_MakeAvailable(doxygen-awesome-css)