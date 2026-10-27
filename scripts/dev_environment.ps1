# MSVC x64 개발자 환경(cl.exe, link.exe, 헤더/라이브러리 경로)을 현재 PowerShell 세션에 불러온다.
# 반드시 dot-source로 실행해야 환경이 유지된다:  . .\scripts\dev_environment.ps1
# Ninja 프리셋은 cl.exe를 직접 호출하므로 빌드 전에 필요하다.

# 여기서 $ErrorActionPreference를 바꾸지 않는다. 이 파일은 dot-source되므로
# 'Stop'이 호출한 쪽에 남아 외부 도구의 stderr 출력까지 치명적 오류로 바뀌기 때문이다.

# 이미 불러온 세션이면 아무것도 하지 않는다.
if (Get-Command cl.exe -ErrorAction SilentlyContinue) {
    Write-Host 'MSVC environment already loaded.'
    return
}

# vswhere: 설치된 Visual Studio를 찾아 주는 공식 도구 (Installer와 함께 설치됨)
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (-not (Test-Path $vswhere)) {
    throw 'vswhere.exe not found. Install Visual Studio 2022 Build Tools with the C++ workload (see docs/BUILD.md).'
}

# x64 C++ 컴파일러가 포함된 가장 최신 설치본을 찾는다.
$vsPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vsPath) {
    throw 'No Visual Studio installation with the MSVC x64 toolset was found.'
}

# VsDevCmd는 PATH에서 vswhere.exe를 찾는데, 없으면 오류 메시지를 출력한다.
$env:Path = (Split-Path $vswhere) + ';' + $env:Path

# Visual Studio의 DevShell 모듈로 64비트 호스트·타깃 환경을 불러온다.
Import-Module (Join-Path $vsPath 'Common7\Tools\Microsoft.VisualStudio.DevShell.dll') -ErrorAction Stop
Enter-VsDevShell -VsInstallPath $vsPath -SkipAutomaticLocation -DevCmdArguments '-arch=x64 -host_arch=x64' -ErrorAction Stop | Out-Null

# 최종 확인: cl.exe가 PATH에 있어야 한다.
if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
    throw 'Developer shell loaded but cl.exe is still not on PATH.'
}
Write-Host "MSVC environment loaded from $vsPath"
