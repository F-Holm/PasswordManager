#include <gtest/gtest.h>
#include <wx/wx.h>

// Creamos una clase de aplicación mínima para wxWidgets
class MyApp : public wxApp {
 public:
  virtual bool OnInit() override { return true; }
};

// Registramos la aplicación wxWidgets
wxIMPLEMENT_APP_NO_MAIN(MyApp);

// Test unitario mínimo para comprobar wxWidgets
TEST(wxWidgetsTest, CanCreateApp) {
  int argc = 0;
  char** argv = nullptr;

  // Inicializamos la aplicación wxWidgets
  MyApp* app = new MyApp();
  bool init_result = app->OnInit();
  delete app;

  // Verificamos que la inicialización fue exitosa
  EXPECT_TRUE(init_result);
}
