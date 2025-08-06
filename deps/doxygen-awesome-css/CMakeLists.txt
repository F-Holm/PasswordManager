set(DOXYGEN_AWESOME_CSS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/deps/doxygen-awesome-css/doxygen-awesome-css-src")

verify_submodule_cloned("doxygen-awesome-css" "${DOXYGEN_AWESOME_CSS_SOURCE_DIR}" "README.md")

include(FetchContent)
FetchContent_Declare(
  doxygen-awesome-css
  SOURCE_DIR "${DOXYGEN_AWESOME_CSS_SOURCE_DIR}"
)
FetchContent_MakeAvailable(doxygen-awesome-css)