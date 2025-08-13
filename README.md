# Gestor de contraseñas

Este proyecto está diseñado para compilarse solo en `Linux x64`<br>
Para compilarlo en otro sistema operativo recomiendo utilizar `Docker`, `WSL`, `Dev Containers` o una `máquina virtual`<br>
Para evitar instalar todas las dependencias se puede utilizar `dev containers`

---

## Targets disponibles:

- wf-linux-x64-release-clang
- wf-linux-x64-release-gcc
- wf-linux-x64-debug-clang
- wf-linux-x64-debug-gcc
- wf-linux-x86-release-gcc
- wf-linux-x86-debug-gcc
- wf-linux-arm64-release-gcc
- wf-linux-arm64-debug-gcc
- wf-linux-armv7-release-gcc
- wf-linux-armv7-debug-gcc

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
- `lcov`
- `cppcheck`
- `graphviz`
- `ccache`
- `gcc`
- `g++`
- `gcc-i686-linux-gnu` (también para clang)
- `g++-i686-linux-gnu` (también para clang++)
- `gcc-arm-linux-gnueabihf` (también para clang)
- `g++-arm-linux-gnueabihf` (también para clang++)
- `gcc-aarch64-linux-gnu` (también para clang)
- `g++-aarch64-linux-gnu` (también para clang++)
- `llvm-mingw`

Para ver como se instalan las dependencias en la imagen docker: [`Dockerfile`](.devcontainer/Dockerfile)