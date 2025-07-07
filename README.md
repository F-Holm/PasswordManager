# Gestor de contraseñas

## Dependencias:
- `CMake`
- `ninja`
- `Visual Studio`
- `make` (Cualquier implementación (`nmake`-`make`))
- `nasm` (Windows)
- `perl`
- `clang`
- `clang++`
- `clang-format`
- `clang-tidy`
- `valgrind` (Linux)
- `doxygen`
- `llvm-cov`
- `graphviz`

---
## Instalación:

### Windows
```cmd
winget install --id LLVM.LLVM --source winget
```
```cmd
winget install --id Kitware.CMake --source winget
```
```cmd
winget install --id DimitriVanHeesch.Doxygen --source winget
```
```cmd
winget install --id Graphviz.Graphviz --source winget
```
```cmd
winget install --id Ninja-build.Ninja --source winget
```
```cmd
winget install --id StrawberryPerl.StrawberryPerl --source winget
```
```cmd
winget install --id NASM.NASM --source winget
```
#### También hay que agregar las direcciones de los binarios instalados al path del sistema
### Linux
```bash
sudo apt update
```
```bash
sudo apt install cmake ninja-build clang clang-format clang-tidy valgrind doxygen llvm graphviz perl make
```