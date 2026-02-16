#!/usr/bin/env pwsh

$ErrorActionPreference = "Stop"

$Paths = @(
    "build"
    "install"
    "crypto/Cargo.lock"
    "crypto/target"
    "gui/build"
    "gui/install"
)

foreach ($Path in $Paths) {
    $FullPath = Join-Path -Path $PSScriptRoot -ChildPath $Path

    if (Test-Path -Path $FullPath) {
        Write-Host "Removing: $FullPath" -ForegroundColor Cyan
        Remove-Item -Path $FullPath -Recurse -Force
    }
}
