#include "data_block.hh"

#include <gtest/gtest.h>

#include <cstring>
#include <string>

// Test constructor por defecto
TEST(DataBlockTest, DefaultConstructor) {
  DataBlock db;
  EXPECT_EQ(db.largo, 0);
  EXPECT_EQ(db.str, nullptr);
}

// Test constructor desde std::string
TEST(DataBlockTest, FromStdString) {
  std::string texto = "Hola mundo";
  DataBlock db(texto);

  EXPECT_EQ(db.largo, texto.size());
  ASSERT_NE(db.str, nullptr);
  EXPECT_EQ(std::string(db.str, db.largo), texto);
}

// Test constructor con largo + char*
TEST(DataBlockTest, FromRawPointer) {
  const char *txt = "OpenAI";
  DataBlock db(6, const_cast<char *>(txt));

  EXPECT_EQ(db.largo, 6);
  EXPECT_EQ(std::string(db.str, db.largo), "OpenAI");
}

// Test copy constructor
TEST(DataBlockTest, CopyConstructor) {
  DataBlock original(std::string("GPT-4"));
  DataBlock copia(original);

  EXPECT_EQ(copia.largo, original.largo);
  EXPECT_NE(copia.str, original.str);  // Deep copy
  EXPECT_EQ(std::string(copia.str, copia.largo), "GPT-4");
}

// Test operador de asignación
TEST(DataBlockTest, AssignmentOperator) {
  DataBlock a(std::string("Data"));
  DataBlock b;
  b = a;

  EXPECT_EQ(b.largo, a.largo);
  EXPECT_NE(b.str, a.str);  // Deep copy
  EXPECT_EQ(std::string(b.str, b.largo), "Data");
}

// Test autoasignación segura
TEST(DataBlockTest, SelfAssignment) {
  DataBlock a(std::string("XYZ"));
  a = a;

  EXPECT_EQ(std::string(a.str, a.largo), "XYZ");
}

// Test destructor implícito (no crashea, no hay test explícito)
TEST(DataBlockTest, DestructorSafe) {
  DataBlock *ptr = new DataBlock(std::string("Temporal"));
  delete ptr;  // Esperamos que no haya memory leak ni double free
}
