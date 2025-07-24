#!/bin/bash

set -euo pipefail

# Constantes
SCRIPT_PATH="$(realpath "$0")"
SCRIPT_DIR="$(dirname "$SCRIPT_PATH")"
scripts_dir="/opt/llvm-mingw"
llvm_mingw_dir="$scripts_dir/llvm-mingw"
bin_dir="$llvm_mingw_dir/bin"
dest_dir="/usr/local/bin"
hook_path="/etc/apt/apt.conf.d/99update-llvm-mingw.conf"

# Obtener JSON de GitHub release "nightly"
json=$(sudo curl -s --fail https://api.github.com/repos/mstorsjo/llvm-mingw/releases/tags/nightly)

# Usar script Python para extraer la URL
download_url=$(echo "$json" | sudo "$SCRIPT_DIR/GET-NAME-LLVM-MINGW-RELEASE.py")

# Descargar el archivo
echo "Descargando: $download_url"
sudo curl -LO "$download_url"

# Extraerlo en el directorio correspondiente
echo "Instalando..."
sudo rm -rf "$llvm_mingw_dir"
sudo mkdir -p "$llvm_mingw_dir"
sudo tar -xf ./llvm-mingw-*.tar.xz -C "$llvm_mingw_dir" --strip-components=1

# Lista de programas
programs=(
    aarch64-w64-mingw32-gcc
    aarch64-w64-mingw32uwp-gcc
    arm64ec-w64-mingw32-gcc
    arm64ec-w64-mingw32uwp-gcc
    armv7-w64-mingw32-gcc
    armv7-w64-mingw32uwp-gcc
    i686-w64-mingw32-gcc
    i686-w64-mingw32uwp-gcc
    x86_64-w64-mingw32-gcc
    x86_64-w64-mingw32uwp-gcc
    aarch64-w64-mingw32-g++
    aarch64-w64-mingw32uwp-g++
    arm64ec-w64-mingw32-g++
    arm64ec-w64-mingw32uwp-g++
    armv7-w64-mingw32-g++
    armv7-w64-mingw32uwp-g++
    i686-w64-mingw32-g++
    i686-w64-mingw32uwp-g++
    x86_64-w64-mingw32-g++
    x86_64-w64-mingw32uwp-g++
    aarch64-w64-mingw32-clang
    aarch64-w64-mingw32uwp-clang
    arm64ec-w64-mingw32-clang
    arm64ec-w64-mingw32uwp-clang
    armv7-w64-mingw32-clang
    armv7-w64-mingw32uwp-clang
    i686-w64-mingw32-clang
    i686-w64-mingw32uwp-clang
    x86_64-w64-mingw32-clang
    x86_64-w64-mingw32uwp-clang
    aarch64-w64-mingw32-clang++
    aarch64-w64-mingw32uwp-clang++
    arm64ec-w64-mingw32-clang++
    arm64ec-w64-mingw32uwp-clang++
    armv7-w64-mingw32-clang++
    armv7-w64-mingw32uwp-clang++
    i686-w64-mingw32-clang++
    i686-w64-mingw32uwp-clang++
    x86_64-w64-mingw32-clang++
    x86_64-w64-mingw32uwp-clang++
)

# Agregando LVM-Mingw al PATH del sistema
echo "Creando symlinks en $dest_dir..."

for prog in "${programs[@]}"; do
  src="$bin_dir/$prog"
  dest="$dest_dir/$prog"

  if [[ ! -f "$src" ]]; then
    echo "❌ No se encontró: $src"
    continue
  fi

  if [[ -L "$dest" || -f "$dest" ]]; then
    #echo "⚠️ Ya existe: $dest — saltando"
    continue
  fi

  sudo ln -s "$src" "$dest"
  echo "✅ Symlink creado: $dest → $src"
done

# Copiando scripts
echo "Copiando scripts"
if [[ "$SCRIPT_DIR" != "$scripts_dir" ]]; then
    echo "Copiando archivos a $scripts_dir..."

    # Crear el directorio si no existe
    sudo mkdir -p "$scripts_dir"

    # Copiar el script actual
    sudo cp "$SCRIPT_PATH" "$scripts_dir/"
    sudo cp "$SCRIPT_DIR/GET-NAME-LLVM-MINGW-RELEASE.py" "$scripts_dir/"
fi

# Automatizando actualizaciones
echo "Automatizando actualizaciones"
if [[ -f "$hook_path" ]]; then
    echo "✅ Ya existe el hook en '$hook_path'. No se realiza ninguna acción."
else
    echo "🔧 Creando hook en '$hook_path'..."

    # Crear el hook con contenido
    sudo tee "$hook_path" > /dev/null <<EOF
APT::Update::Post-Invoke { "sudo bash '$scripts_dir/UPDATE-LLVM-MINGW.sh' || true"; };
EOF

    echo "✅ Hook creado exitosamente."
fi

# Eliminar archivos temporales
echo "Eliminando archivos temporales"
sudo rm -f ./llvm-mingw-*.tar.xz
sudo rm -rf ./__pycache__