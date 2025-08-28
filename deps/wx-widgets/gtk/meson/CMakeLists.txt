set(MESON_REPO_DIR "${CMAKE_SOURCE_DIR}/deps/wx-widgets/gtk/meson/meson-src")
set(MESON_SOURCE_DIR "${PROJECT_BINARY_DIR}/_deps/meson-src")

verify_submodule_cloned("Meson" "${MESON_REPO_DIR}" "meson.py")

# Copiando
CheckCopy("${MESON_SOURCE_DIR}")
if (NOT ${CHECK})
    message(STATUS "Copiando Meson...")
    file(MAKE_DIRECTORY "${MESON_SOURCE_DIR}")
    file(COPY "${MESON_REPO_DIR}/" DESTINATION "${MESON_SOURCE_DIR}")
    AddCheckCopy("${MESON_SOURCE_DIR}")
endif()
unset(COPY)

set(MESON "${MESON_SOURCE_DIR}" CACHE STRING "Meson Path")