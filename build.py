#!/usr/bin/env python3
"""
build.py

Ejecuta el workflow de CMake para compilar el proyecto.
"""

import subprocess
import sys

def run_command(cmd):
    """
    Ejecuta un comando y maneja errores.
    """
    try:
        subprocess.run(cmd, check=True, shell=True)
    except subprocess.CalledProcessError as e:
        print(f"[ERROR] Falló el comando: {cmd}", file=sys.stderr)
        sys.exit(e.returncode)

def clear_cache():
    """
    Limpieza de archivos temporales si fuera necesario.
    """
    # Placeholder, en caso de querer eliminar artefactos temporales
    pass

def main():
    run_command("cmake --workflow --preset wf-debug")
    clear_cache()

if __name__ == "__main__":
    main()
