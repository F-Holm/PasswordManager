#include "db.hh"

#include "datablock.hh"

#include <vector>
#include <string>
#include <cstdio>

#include <gtest/gtest.h>

// Helper para comparar dos vectores de DataBlock
bool CompararDataBlocks(const std::vector<DataBlock> &a, const std::vector<DataBlock> &b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i].largo != b[i].largo) return false;
        if (std::string(a[i].str, a[i].largo) != std::string(b[i].str, b[i].largo))
            return false;
    }
    return true;
}

TEST(DBTest, EscribirYLeerArchivoBasico) {
    const std::string archivo = "test_db_basico.bin";

    std::vector<DataBlock> datos_original = {
        DataBlock("hola"),
        DataBlock("mundo"),
        DataBlock("1234567890")
    };

    DB::Escribir(archivo, datos_original);

    std::vector<DataBlock> datos_leidos = DB::Leer(archivo);

    EXPECT_TRUE(CompararDataBlocks(datos_original, datos_leidos));

    std::remove(archivo.c_str());
}

TEST(DBTest, LeerArchivoVacio) {
    const std::string archivo = "test_db_vacio.bin";

    // Crear archivo vacío
    {
        FILE* f = fopen(archivo.c_str(), "wb");
        ASSERT_NE(f, nullptr);
        fclose(f);
    }

    std::vector<DataBlock> datos = DB::Leer(archivo);

    EXPECT_TRUE(datos.empty());

    std::remove(archivo.c_str());
}

TEST(DBTest, LeerArchivoNoExistente) {
    const std::string archivo = "archivo_inexistente.bin";

    // Depende si DB::Leer lanza excepción o devuelve vector vacío
    // Por ejemplo, si lanza excepción:
    EXPECT_THROW({
        auto datos = DB::Leer(archivo);
        (void)datos;
    }, std::exception);

    // O si devuelve vector vacío, usar:
    // EXPECT_TRUE(DB::Leer(archivo).empty());
}

TEST(DBTest, EscribirYLeerDatosConCaracteresEspeciales) {
    const std::string archivo = "test_db_especiales.bin";

    std::vector<DataBlock> datos_original = {
        DataBlock(std::string("abc\n123\t\0xyz", 11)),  // incluye '\0' en medio
        DataBlock(std::string("\xFF\x00\x01", 3))
    };

    DB::Escribir(archivo, datos_original);

    std::vector<DataBlock> datos_leidos = DB::Leer(archivo);

    EXPECT_TRUE(CompararDataBlocks(datos_original, datos_leidos));

    std::remove(archivo.c_str());
}
