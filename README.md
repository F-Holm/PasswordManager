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
- `gcc`
- `clang`
- `clang++`
- `clang-format`
- `clang-tidy`
- `clang-tools`
- `valgrind`
- `doxygen`
- `llvm-cov`
- `graphviz`
- `mingw-w64`
- `ccache`
- `gcc-i686-linux-gnu`
- `g++-i686-linux-gnu`
- `gcc-arm-linux-gnueabihf`
- `g++-arm-linux-gnueabihf`
- `gcc-aarch64-linux-gnu`
- `g++-aarch64-linux-gnu`

Para ver como se instalan las dependencias en la imagen docker: [`Dockerfile`](.devcontainer/Dockerfile)

---

## Instalación:

```bash
sudo apt update && sudo apt install -y build-essential cmake ninja-build clang clang-format clang-tidy clang-tools valgrind doxygen llvm graphviz perl make mingw-w64 gcc ccache gcc-i686-linux-gnu g++-i686-linux-gnu gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf gcc-aarch64-linux-gnu g++-aarch64-linux-gnu
```