include(FetchContent)
FetchContent_Declare(
  wxwidgets
  GIT_REPOSITORY https://github.com/wxWidgets/wxWidgets.git
  GIT_TAG v3.2.9
)
FetchContent_MakeAvailable(wxwidgets)

function(Link_wxWidgets target)
    target_link_libraries(${target} PRIVATE wx::base wx::core)
endfunction()
