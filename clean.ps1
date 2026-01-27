#!/usr/bin/env pwsh

Remove-Item -Path "./build"   -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item -Path "./install" -Recurse -Force -ErrorAction SilentlyContinue
git clean -Xfd
