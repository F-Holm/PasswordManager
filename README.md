# Gestor de contraseñas

Este proyecto está diseñado para compilarse solo en `Linux x64`<br>
Para compilarlo en otro sistema operativo recomiendo utilizar `Docker`, `WSL`, `Dev Containers` o una `máquina virtual`<br>
Para evitar instalar todas las dependencias se puede utilizar `dev containers`

---

## Dependencias:
- `Docker`
- `Visual Studio Code` + extensión `Dev Containers` **o** CLI compatible (`devcontainer up`)
- Acceso a internet para construir la imagen

---

## Dependencias sin virtualización:
- `CMake`
- `ninja`
- `make`
- `perl`
- `clang`
- `clang++`
- `clang-format`
- `clang-tidy`
- `clang-tools`
- `valgrind`
- `doxygen`
- `llvm-cov`
- `graphviz`
- `ccache`
- `gcc`
- `g++`
- `gcc-i686-linux-gnu`
- `g++-i686-linux-gnu`
- `gcc-arm-linux-gnueabihf`
- `g++-arm-linux-gnueabihf`
- `gcc-aarch64-linux-gnu`
- `g++-aarch64-linux-gnu`
- `llvm-mingw`

Para ver como se instalan las dependencias en la imagen docker: [`Dockerfile`](.devcontainer/Dockerfile)

---

## Instalación:

```bash
sudo apt update && sudo apt install -y build-essential cmake ninja-build make perl clang clang-format clang-tidy clang-tools llvm valgrind doxygen graphviz ccache gcc g++ gcc-i686-linux-gnu g++-i686-linux-gnu gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf gcc-aarch64-linux-gnu g++-aarch64-linux-gnu
```