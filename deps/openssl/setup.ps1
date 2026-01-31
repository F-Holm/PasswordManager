#!/usr/bin/env pwsh

$OpenSSLVersion = "3.6.1"
$OpenSSLName    = "openssl-$OpenSSLVersion"
$Url            = "https://github.com/openssl/openssl/releases/download/$OpenSSLName/$OpenSSLName.tar.gz"

$SrcDir        = "$PSScriptRoot/$OpenSSLName"
$Tarball       = Join-Path $SrcDir "$OpenSSLName.tar.gz"
$InstallDir    = Join-Path $SrcDir "$PSScriptRoot/../../lib/openssl/"
$LlvmMingwBinPath = "/opt/llvm-mingw/bin"

Set-Location $PSScriptRoot

Write-Host "==> OpenSSL version: $OpenSSLVersion"
Write-Host "==> Working dir: $SrcDir"

# -------------------------------------------------------------------
# Download
# -------------------------------------------------------------------

if (-not (Test-Path $Tarball)) {
    Write-Host "==> Downloading OpenSSL..."
    Invoke-WebRequest -Uri $Url -OutFile $Tarball
} else {
    Write-Host "==> Tarball already exists, skipping download"
}

# -------------------------------------------------------------------
# Extraction
# -------------------------------------------------------------------

if (Test-Path $SrcDir) {
    Write-Host "==> Removing previous source directory"
    Remove-Item -Recurse -Force $SrcDir
}

Write-Host "==> Extracting OpenSSL..."
tar -xzf $Tarball -C $SrcDir

Set-Location $SrcDir

# -------------------------------------------------------------------
# Configuration and Build
# -------------------------------------------------------------------

# SO | Arch | Compiler | Compiler Prefix | UCRT Flags | Target

$configs = @(
    @{ OS="linux";   Arch="x64";   CC="clang"; Prefix="";                                      UCRT=0; Target="linux-x86_64-clang" }
    #@{ OS="linux";   Arch="x86";   CC="gcc";   Prefix="$LlvmMingwBinPath/i686-linux-gnu-";     UCRT=0; Target="linux-x86" },
    #@{ OS="linux";   Arch="arm64"; CC="gcc";   Prefix="$LlvmMingwBinPath/aarch64-linux-gnu-";  UCRT=0; Target="linux-aarch64" },
    #@{ OS="windows"; Arch="x64";   CC="clang"; Prefix="$LlvmMingwBinPath/x86_64-w64-mingw32-"; UCRT=1; Target="mingw64" },
    #@{ OS="windows"; Arch="x86";   CC="clang"; Prefix="$LlvmMingwBinPath/i686-w64-mingw32-";   UCRT=1; Target="mingw" },
    #@{ OS="windows"; Arch="arm64"; CC="clang"; Prefix="$LlvmMingwBinPath/aarch64-w64-mingw32-";UCRT=1; Target="mingwarm64" }
)

$Jobs      = [Environment]::ProcessorCount
$BaseFlags = 'shared no-tests no-apps no-docs'

foreach ($cfg in $configs) {
    Write-Host "==> Building $($cfg.OS)-$($cfg.Arch)"
    
    $env:CC = $cfg.CC
    if ($cfg.UCRT -eq 1) {
        $env:CFLAGS   = "-D__USE_MINGW_ANSI_STDIO=1"
        $env:CXXFLAGS = $env:CFLAGS
    } else {
        $env:CFLAGS   = ""
        $env:CXXFLAGS = ""
    }
    
    "perl Configure $CrossCompilerPrefix$matrix[$i,3] $matrix[$i,5] $BaseFlags --prefix=$InstallDir$matrix[$i,0]-$matrix[$i,1]"

    perl Configure --cross-compile-prefix=$($cfg.Prefix) $($cfg.Target) $BaseFlags --prefix="$InstallDir/$($cfg.OS)-$($cfg.Arch)"
    make -j$Jobs
    make install_sw
    make clean
}
