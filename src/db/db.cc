#include "db.hh"

#include <fstream>

using std::ifstream;
using std::ofstream;
using std::string;
using std::vector;

vector<DataBlock> DB::leer(const string nom_arch) {
  vector<DataBlock> datos;
  ifstream archivo(nombreArchivo, std::ios::binary);
  if (!archivo)
    return datos;

  while (archivo) {
    size_t largo;
    archivo.read(reinterpret_cast<char *>(&largo), sizeof(size_t));
    if (!archivo)
      break;

    unsigned char *str = new unsigned char[largo];
    archivo.read(reinterpret_cast<char *>(str), largo);
    if (!archivo) {
      delete[] str;
      break;
    }

    datos.emplace_back(largo, str);
    delete[] str;
  }
  return datos;
}
void DB::escribir(const string nombreArchivo, vector<DataBlock> datos) {
  ofstream archivo(nombreArchivo, std::ios::binary);
  if (!archivo)
    return;

  for (DataBlock dato : datos) {
    archivo.write(reinterpret_cast<const char *>(&dato.largo), sizeof(size_t));
    archivo.write(reinterpret_cast<const char *>(dato.str), dato.largo);
  }

  archivo.close();
}