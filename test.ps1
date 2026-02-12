#!/usr/bin/env pwsh

$ErrorActionPreference = "Stop"

$Tasks = @(
    @{ Path = "crypto"; Exec = "cargo"; Args = "fmt" }
    @{ Path = "crypto"; Exec = "cargo"; Args = "clippy" }
    @{ Path = "crypto"; Exec = "cargo"; Args = "test" }
    @{ Path = "core";   Exec = "cmake"; Args = "--workflow", "--preset", "wf-debug" }
)

$SuccessCount = 0

foreach ($Task in $Tasks) {
    $TargetDir = Join-Path -Path $PSScriptRoot -ChildPath $Task.Path

    if (Test-Path -Path $TargetDir) {
        Write-Host "`n[Task] Executing in: $($Task.Path)" -ForegroundColor Cyan
        
        Push-Location $TargetDir
        try {
            & $Task.Exec $Task.Args
            
            if ($LASTEXITCODE -ne 0) {
                Write-Host "`n[Error] Task failed in $($Task.Path). Aborting." -ForegroundColor Red
                exit $LASTEXITCODE
            }
            
            $SuccessCount++
        }
        finally {
            Pop-Location
        }
    }
    else {
        Write-Error "Directory not found: $TargetDir"
    }
}

Write-Host "`nSummary: $SuccessCount/$($Tasks.Count) tasks passed." -ForegroundColor Green
