import subprocess
import sys
import shutil
from pathlib import Path

def remove_dir(path):
    """
    Elimina un directorio si existe.
    """
    p = Path(path)
    if p.exists() and p.is_dir():
        shutil.rmtree(p)
        print(f"[INFO] Eliminado: {path}")

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
    remove_dir("./__pycache__")
    pass