# Gestor de contraseñas

## Dependencias:
- `CMake`
- `ninja`
- `clang`
- `clang++`
- `clang-format`
- `clang-tidy`
- `clang-tools`
- `valgrind` (Linux)
- `memcheck` (Linux)
- `doxygen`
- `llvm-cov`
- `cppcheck`
- `graphviz`
- `ccache`
- `openssl`

## Instalar dependencias

### Linux Ubuntu

```bash
sudo apt update
```
```bash
sudo apt install \
    cmake \
    ninja-build \
    clang \
    clang-format \
    clang-tidy \
    clang-tools \
    llvm \
    cppcheck \
    valgrind \
    doxygen \
    graphviz \
    ccache \
    libssl-dev
```

### Windows 10/11

```powershell
winget install --id Kitware.CMake --source winget
```
```powershell
winget install --id LLVM.LLVM --source winget
```
```powershell
winget install --id DimitriVanHeesch.Doxygen --source winget
```
```powershell
winget install --id Cppcheck.Cppcheck --source winget
```
```powershell
winget install --id Graphviz.Graphviz --source winget
```
```powershell
winget install --id Ccache.Ccache --source winget
```
```powershell
winget install --id ShiningLight.OpenSSL.Dev --source winget
```