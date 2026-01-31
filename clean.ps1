#!/usr/bin/env pwsh

Remove-Item -Recurse -Force ./build   -ErrorAction SilentlyContinue
Remove-Item -Recurse -Force ./install -ErrorAction SilentlyContinue
#Remove-Item -Recurse -Force ./lib     -ErrorAction SilentlyContinue
#git clean -Xfd
