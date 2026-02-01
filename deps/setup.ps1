#!/usr/bin/env pwsh

Set-Location $PSScriptRoot
$LibDir = Join-Path $PSScriptRoot "../lib"
#Remove-Item -Recurse -Force $LibDir

#& "$PSScriptRoot/openssl/setup.ps1"
& "$PSScriptRoot/wxwidgets/setup.ps1"
