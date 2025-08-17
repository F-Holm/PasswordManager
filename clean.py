#!/usr/bin/env python3
"""
clean.py

Elimina los directorios de build e install y limpia archivos generados por Git.
"""

from base import remove_dir, run_command, clear_cache

def main():
    remove_dir("./build")
    remove_dir("./install")
    run_command("git clean -Xfd")
    clear_cache()

if __name__ == "__main__":
    main()