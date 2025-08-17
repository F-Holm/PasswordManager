#!/usr/bin/env python3
"""
reset_modules.py

Resetea todos los submódulos a su estado inicial.
"""

from base import run_command, clear_cache

def main():
    run_command("git submodule update -f --init --recursive --checkout")
    clear_cache()

if __name__ == "__main__":
    main()