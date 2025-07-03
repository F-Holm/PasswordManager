# Gestor de contraseñas

## Dependencias:
- `CMake`
- `ninja`
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
### Linux
```bash
sudo apt update
```
```bash
sudo apt install cmake ninja-build clang clang-format clang-tidy valgrind doxygen llvm graphviz 
```