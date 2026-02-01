#!/usr/bin/env pwsh

$ErrorActionPreference = "Stop"

$WxWidgetsVersion = "3.2.9"
$WxWidgetsName    = "wxWidgets-$WxWidgetsVersion"
$Url            = "https://github.com/wxWidgets/wxWidgets/releases/download/v$WxWidgetsVersion/$WxWidgetsName.tar.bz2"

$SrcDir        = Join-Path $PSScriptRoot $WxWidgetsName
$Tarball       = Join-Path $PSScriptRoot "$WxWidgetsName.tar.bz2"
$InstallDir    = Join-Path $PSScriptRoot "../../lib/wxwidgets/"
$LlvmMingwBinPath = "/opt/llvm-mingw/bin"

Set-Location $PSScriptRoot

Write-Host "==> wxWidgets version: $WxWidgetsVersion"
Write-Host "==> Working dir: $PSScriptRoot"

# -------------------------------------------------------------------
# Download
# -------------------------------------------------------------------

if (-not (Test-Path $Tarball)) {
    Write-Host "==> Downloading wxWidgets..."
    Invoke-WebRequest -Uri $Url -OutFile $Tarball
} else {
    Write-Host "==> Tarball already exists, skipping download"
}

# -------------------------------------------------------------------
# Extraction
# -------------------------------------------------------------------

if (Test-Path $SrcDir) {
    throw "Delete $SrcDir"
}

Write-Host "==> Extracting wxWidgets..."
tar -xjf $Tarball -C $PSScriptRoot

Set-Location $SrcDir
Write-Host "==> Working dir: $SrcDir"

# -------------------------------------------------------------------
# Configuration and Build
# -------------------------------------------------------------------

$configs = @(
    @{ OS="Linux";   Arch="x86_64";  CC="clang"; CXXC="clang++"; Prefix="" },
    #@{ OS="Linux";   Arch="x86";     CC="gcc";   CC="g++";       Prefix="i686-linux-gnu-" },
    #@{ OS="Linux";   Arch="aarch64"; CC="gcc";   CC="g++";       Prefix="aarch64-linux-gnu-" },
    @{ OS="Windows"; Arch="x86_64";  CC="clang"; CXXC="clang++"; Prefix="$LlvmMingwBinPath/x86_64-w64-mingw32-" },
    @{ OS="Windows"; Arch="x86";     CC="clang"; CXXC="clang++"; Prefix="$LlvmMingwBinPath/i686-w64-mingw32-" },
    @{ OS="Windows"; Arch="aarch64"; CC="clang"; CXXC="clang++"; Prefix="$LlvmMingwBinPath/aarch64-w64-mingw32-" }
)

foreach ($cfg in $configs) {
    Write-Host "==> Building $($cfg.OS)-$($cfg.Arch)"

    $BuildDir = Join-Path $InstallDir "$($cfg.OS)-$($cfg.Arch)"
    New-Item -Path $BuildDir -ItemType Directory
    Set-Location $BuildDir

    $ExtraFlags = ""
    if ($($cfg.OS) -eq "Linux" -and $($cfg.Arch) -ne "x86_64"){
        $ExtraFlags = "$ExtraFlags -DCMAKE_SYSROOT=$HOME/cross-env/sysroot-arm64 -DCMAKE_FIND_ROOT_PATH=$HOME/cross-env/sysroot-arm64"
    }

    cmake $SrcDir -G Ninja `
        "-DCMAKE_C_COMPILER=$($cfg.Prefix)$($cfg.CC)" `
        "-DCMAKE_CXX_COMPILER=$($cfg.Prefix)$($cfg.CXXC)" `
        "-DCMAKE_SYSTEM_NAME=$($cfg.OS)" `
        "-DCMAKE_SYSTEM_PROCESSOR=$($cfg.Arch)" `
        "-DwxBUILD_SHARED=OFF" `
        "-DCMAKE_BUILD_TYPE=Release" `
        $ExtraFlags

    ninja
}

# -------------------------------------------------------------------
# Clean
# -------------------------------------------------------------------

Set-Location $PSScriptRoot
Remove-Item -Recurse -Force $SrcDir
Remove-Item -Force $Tarball
