# Dependencias
add_subdirectory(gtk)

# Build
set(WX_WIDGETS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/wx-widgets-src")

verify_submodule_cloned("wxWidgets" "${WX_WIDGETS_SOURCE_DIR}" "CMakeLists.txt")

include(FetchContent)
FetchContent_Declare(
  wx-widgets
  SOURCE_DIR "${WX_WIDGETS_SOURCE_DIR}"
)
FetchContent_MakeAvailable(wx-widgets)
