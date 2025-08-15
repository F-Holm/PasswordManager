#!/usr/bin/env python3
"""
reset_modules.py

Resetea todos los submódulos a su estado inicial.
"""

import subprocess
import sys

def run_command(cmd):
    try:
        subprocess.run(cmd, check=True, shell=True)
    except subprocess.CalledProcessError as e:
        print(f"[ERROR] Falló el comando: {' '.join(cmd)}", file=sys.stderr)
        sys.exit(e.returncode)

def clear_cache():
    """
    Limpia posibles archivos temporales o caché si existieran.
    """
    pass

def main():
    run_command("git submodule update -f --init --recursive --checkout")
    clear_cache()

if __name__ == "__main__":
    main()
