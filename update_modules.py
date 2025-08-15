#!/usr/bin/env python3
"""
update_modules.py

Actualiza todos los submódulos de Git del proyecto, sincronizándolos y trayendo cambios remotos.
"""

import subprocess
import sys

def run_command(cmd):
    """
    Ejecuta un comando en el shell y maneja errores.
    """
    try:
        subprocess.run(cmd, check=True, shell=True)
    except subprocess.CalledProcessError as e:
        print(f"[ERROR] Falló el comando: {' '.join(cmd)}", file=sys.stderr)
        sys.exit(e.returncode)

def clear_cache():
    """
    Limpia posibles archivos temporales o caché si existieran.
    """
    # En este caso, no hay caché específico, solo placeholder
    pass

def main():
    run_command("git submodule sync")
    run_command("git submodule update -f --init --recursive --checkout")
    run_command("git submodule update -f --remote --recursive")
    clear_cache()

if __name__ == "__main__":
    main()
