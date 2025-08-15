add_executable(wx-widgets_test
    wx-widgets_test.cc
)

LinkWxWidgets(wx-widgets_test)

AddTests(wx-widgets_test)

Format(wx-widgets_test .)