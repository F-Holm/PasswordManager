#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

PATHS=(
  "build"
  "install"
  "core/build"
  "core/install"
  "crypto/Cargo.lock"
  "crypto/target"
  "gui/build"
  "gui/install"
  "cli/build"
  "cli/install"
)

for path in "${PATHS[@]}"; do
  FULL_PATH="$SCRIPT_DIR/$path"
  
  if [ -e "$FULL_PATH" ]; then
    echo "Removing: $FULL_PATH"
    rm -rf "$FULL_PATH"
  #else
    #echo "Not Found: $FULL_PATH"
  fi
done
