# Runs all registered tests for an already-built preset.
# Usage: .\scripts\run_tests.ps1 [-Preset windows-debug|windows-release]

param(
    [ValidateSet('windows-debug', 'windows-release')]
    [string]$Preset = 'windows-debug'
)

$ErrorActionPreference = 'Stop'
Set-Location (Join-Path $PSScriptRoot '..')

ctest --preset $Preset
exit $LASTEXITCODE
