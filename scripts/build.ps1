# Configures and builds all targets.
# Usage: .\scripts\build.ps1 [-Preset windows-debug|windows-release]

param(
    [ValidateSet('windows-debug', 'windows-release')]
    [string]$Preset = 'windows-debug'
)

$ErrorActionPreference = 'Stop'
Set-Location (Join-Path $PSScriptRoot '..')
. (Join-Path $PSScriptRoot 'dev_environment.ps1')

cmake --preset $Preset
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

cmake --build --preset $Preset
exit $LASTEXITCODE
