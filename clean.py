#!/usr/bin/env python3
"""
clean.py

Elimina los directorios de build e install y limpia archivos generados por Git.
"""

import shutil
from pathlib import Path
import subprocess
import sys

def remove_dir(path):
    """
    Elimina un directorio si existe.
    """
    p = Path(path)
    if p.exists() and p.is_dir():
        shutil.rmtree(p)
        print(f"[INFO] Eliminado: {path}")

def run_command(cmd):
    try:
        subprocess.run(cmd, check=True, shell=True)
    except subprocess.CalledProcessError as e:
        print(f"[ERROR] Falló el comando: {' '.join(cmd)}", file=sys.stderr)
        sys.exit(e.returncode)

def clear_cache():
    """
    Limpieza de cache temporal adicional si fuera necesario.
    """
    # En este caso, placeholder
    pass

def main():
    remove_dir("./build")
    remove_dir("./install")
    run_command("git clean -Xfd")
    clear_cache()

if __name__ == "__main__":
    main()
