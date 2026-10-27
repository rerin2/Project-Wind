# Starts the game server. Extra arguments are passed through, e.g.
#   .\scripts\run_server.ps1 --log-level debug

[CmdletBinding(PositionalBinding = $false)]
param(
    [ValidateSet('windows-debug', 'windows-release')]
    [string]$Preset = 'windows-debug',
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$ServerArgs
)

$ErrorActionPreference = 'Stop'
$exe = Join-Path $PSScriptRoot "..\build\$Preset\bin\wind_server.exe"
if (-not (Test-Path $exe)) {
    throw "Server not built: $exe. Run .\scripts\build.ps1 -Preset $Preset first."
}

& $exe @ServerArgs
exit $LASTEXITCODE
