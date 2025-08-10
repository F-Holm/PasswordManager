#!/bin/bash
set -e

###############################################################################
# Script de compilación de workflows de CMake
#
# FLAGS DISPONIBLES:
#
# Targets:
#   -a / -A  -> Ejecuta todos los workflows (Linux y Windows)
#   -l / -L  -> Ejecuta workflows de Linux
#   -w / -W  -> Ejecuta workflows de Windows
#
# Modos de compilación:
#   -r / -R  -> Release
#   -d / -D  -> Debug
#
# Arquitecturas individuales:
#   -x64     -> x64
#   -x86     -> x86
#   -arm64   -> ARM64
#   -armv7   -> ARMv7
#
# Arquitecturas grupales:
#   -x       -> x64 y x86
#   -arm     -> ARM64 y ARMv7
#   -64      -> x64 y ARM64
#   -32      -> x86 y ARMv7
#
# Compiladores:
#   -c       -> clang
#   -g       -> gcc
#
# REGLAS DE COMPATIBILIDAD:
#   - Solo un modo de compilación (-r o -d).
#   - Se pueden combinar sistemas libremente.
#   - Las arquitecturas pueden combinarse si son todas individuales,
#     o si se usa un solo grupo, pero no mezclar un grupo con individuales
#     incompatibles.
#   - Los compiladores se pueden combinar libremente.
#
# Ejemplos:
#   ./build.sh -l -r -x64 -c
#   ./build.sh -a -d -arm -g
#   ./build.sh -w -r -x64 -armv7 -c -g
###############################################################################

# --- Variables ---
declare -a SYSTEMS
declare -a MODES
declare -a ARCHS
declare -a COMPS
declare -a WORKFLOWS
used_group_arch=false

# --- Funciones ---
error_exit() {
    echo "Error: $1" >&2
    exit 1
}

add_unique() {
    local arrname=$1
    local value=$2
    eval "arr=(\"\${$arrname[@]}\")"
    for v in "${arr[@]}"; do
        [[ "$v" == "$value" ]] && return
    done
    eval "$arrname+=(\"$value\")"
}

show_help() {
    sed -n '4,67p' "$0"
}

# Si no se pasa ninguna flag → mostrar ayuda y salir
if [[ $# -eq 0 ]]; then
    show_help
    exit 0
fi

# --- Procesar flags ---
while [[ $# -gt 0 ]]; do
    case "$1" in
        -a|-A) add_unique SYSTEMS "linux"; add_unique SYSTEMS "windows" ;;
        -l|-L) add_unique SYSTEMS "linux" ;;
        -w|-W) add_unique SYSTEMS "windows" ;;
        -r|-R) add_unique MODES "release" ;;
        -d|-D) add_unique MODES "debug" ;;
        -c)    add_unique COMPS "clang" ;;
        -g)    add_unique COMPS "gcc" ;;
        -x64)   [[ "$used_group_arch" == true ]] && error_exit "No se puede mezclar flags grupales con individuales incompatibles."
                 add_unique ARCHS "x64" ;;
        -x86)   [[ "$used_group_arch" == true ]] && error_exit "No se puede mezclar flags grupales con individuales incompatibles."
                 add_unique ARCHS "x86" ;;
        -arm64) [[ "$used_group_arch" == true ]] && error_exit "No se puede mezclar flags grupales con individuales incompatibles."
                 add_unique ARCHS "arm64" ;;
        -armv7) [[ "$used_group_arch" == true ]] && error_exit "No se puede mezclar flags grupales con individuales incompatibles."
                 add_unique ARCHS "armv7" ;;
        -x)     [[ ${#ARCHS[@]} -gt 0 ]] && error_exit "No se puede mezclar grupo -x con otras arquitecturas."
                 ARCHS=("x64" "x86"); used_group_arch=true ;;
        -arm)   [[ ${#ARCHS[@]} -gt 0 ]] && error_exit "No se puede mezclar grupo -arm con otras arquitecturas."
                 ARCHS=("arm64" "armv7"); used_group_arch=true ;;
        -64)    [[ ${#ARCHS[@]} -gt 0 ]] && error_exit "No se puede mezclar grupo -64 con otras arquitecturas."
                 ARCHS=("x64" "arm64"); used_group_arch=true ;;
        -32)    [[ ${#ARCHS[@]} -gt 0 ]] && error_exit "No se puede mezclar grupo -32 con otras arquitecturas."
                 ARCHS=("x86" "armv7"); used_group_arch=true ;;
        *) error_exit "Flag desconocida: $1" ;;
    esac
    shift
done

# --- Validaciones ---
[[ ${#MODES[@]} -gt 1 ]] && error_exit "No se pueden combinar más de un modo (-r/-d)."

# --- Defaults si no se especifica algo ---
[[ ${#SYSTEMS[@]} -eq 0 ]] && SYSTEMS=("linux" "windows")
[[ ${#MODES[@]} -eq 0 ]] && MODES=("debug" "release")
[[ ${#ARCHS[@]} -eq 0 ]] && ARCHS=("x64" "x86" "arm64" "armv7")
[[ ${#COMPS[@]} -eq 0 ]] && COMPS=("clang" "gcc")

# --- Limpiar ---
./clean.sh
ccache -C > /dev/null 2>&1

# --- Generar workflows ---
for sys in "${SYSTEMS[@]}"; do
    for arch in "${ARCHS[@]}"; do
        for mode in "${MODES[@]}"; do
            for cc in "${COMPS[@]}"; do
                preset="wf-${sys}-${arch}-${mode}-${cc}"
                WORKFLOWS+=("$preset")
            done
        done
    done
done

# --- Ejecutar workflows ---
for wf in "${WORKFLOWS[@]}"; do
    echo "Ejecutando $wf..."
    cmake --workflow --preset "$wf"
    ccache -C > /dev/null 2>&1
done
