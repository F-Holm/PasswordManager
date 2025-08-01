#include "db.hh"

#include <fstream>

auto DB::Leer(const std::string &nom_arch) -> std::vector<DataBlock> {
  std::vector<DataBlock> datos;
  std::ifstream arch(nom_arch, std::ios::binary);
  if (!arch) {
    return datos;
  }

  while (arch) {
    size_t largo;
    arch.read(reinterpret_cast<char *>(&largo), sizeof(size_t));
    if (!arch) {
      break;
  }

    char *str = new char[largo];
    arch.read(reinterpret_cast<char *>(str), largo);
    if (!arch) {
      delete[] str;
      break;
    }

    datos.emplace_back(largo, str);
    delete[] str;
  }
  return datos;
}

void DB::Escribir(const std::string& nom_arch, const std::vector<DataBlock> &datos) {
  std::ofstream arch(nom_arch, std::ios::binary);
  if (!arch) {
    return;
  }

  for (DataBlock dato : datos) {
    arch.write(reinterpret_cast<const char *>(&dato.largo), sizeof(size_t));
    arch.write(reinterpret_cast<const char *>(dato.str), dato.largo);
  }

  arch.close();
}