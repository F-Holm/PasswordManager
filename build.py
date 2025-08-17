#!/usr/bin/env python3
"""
build.py

Ejecuta el workflow de CMake para compilar el proyecto.
"""

from base import run_command, clear_cache

def main():
    run_command("cmake --workflow --preset wf-linux-x64-debug-clang")
    clear_cache()

if __name__ == "__main__":
    main()