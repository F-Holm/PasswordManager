#!/usr/bin/env python3
"""
update_modules.py

Actualiza todos los submódulos de Git del proyecto, sincronizándolos y trayendo cambios remotos.
"""

from base import run_command, clear_cache

def main():
    run_command("git submodule sync")
    run_command("git submodule update -f --init --recursive --checkout")
    run_command("git submodule update -f --remote --recursive")
    clear_cache()

if __name__ == "__main__":
    main()