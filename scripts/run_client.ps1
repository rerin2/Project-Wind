# Starts the game client. Extra arguments are passed through, e.g.
#   .\scripts\run_client.ps1 --width 1600 --height 900

[CmdletBinding(PositionalBinding = $false)]
param(
    [ValidateSet('windows-debug', 'windows-release')]
    [string]$Preset = 'windows-debug',
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$ClientArgs
)

$ErrorActionPreference = 'Stop'
$exe = Join-Path $PSScriptRoot "..\build\$Preset\bin\wind_client.exe"
if (-not (Test-Path $exe)) {
    throw "Client not built: $exe. Run .\scripts\build.ps1 -Preset $Preset first."
}

& $exe @ClientArgs
exit $LASTEXITCODE
