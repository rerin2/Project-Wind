# Loads the MSVC x64 developer environment into the current PowerShell session.
# Dot-source it:  . .\scripts\dev_environment.ps1
# Required for the Ninja presets, which call cl.exe directly.

# No $ErrorActionPreference change here: this file is dot-sourced, and 'Stop'
# would leak into the caller and turn native tools' stderr into fatal errors.

if (Get-Command cl.exe -ErrorAction SilentlyContinue) {
    Write-Host 'MSVC environment already loaded.'
    return
}

$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (-not (Test-Path $vswhere)) {
    throw 'vswhere.exe not found. Install Visual Studio 2022 Build Tools with the C++ workload (see docs/BUILD.md).'
}

$vsPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vsPath) {
    throw 'No Visual Studio installation with the MSVC x64 toolset was found.'
}

# VsDevCmd looks up vswhere.exe on PATH and prints an error if it is missing.
$env:Path = (Split-Path $vswhere) + ';' + $env:Path

Import-Module (Join-Path $vsPath 'Common7\Tools\Microsoft.VisualStudio.DevShell.dll') -ErrorAction Stop
Enter-VsDevShell -VsInstallPath $vsPath -SkipAutomaticLocation -DevCmdArguments '-arch=x64 -host_arch=x64' -ErrorAction Stop | Out-Null

if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
    throw 'Developer shell loaded but cl.exe is still not on PATH.'
}
Write-Host "MSVC environment loaded from $vsPath"
