# Gestor de contraseñas

## Sistemas Operativos compatibles

- `Linux x64`
- `Windows 10-11 x64`

---

## Dependencias:
- `CMake`
- `ninja`
- `Visual Studio Build Tools 2022` (Windows)
- `nasm` (Windows)
- `make` (Linux)
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
```cmd
winget install --id Microsoft.VisualStudio.2022.BuildTools --source winget
```
#### También hay que agregar las direcciones de los binarios instalados al path del sistema
### Linux
```bash
sudo apt update
```
```bash
sudo apt install cmake ninja-build clang clang-format clang-tidy valgrind doxygen llvm graphviz perl make
```