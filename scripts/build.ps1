# 모든 타깃을 configure하고 빌드한다.
# 사용법: .\scripts\build.ps1 [-Preset windows-debug|windows-release]

param(
    [ValidateSet('windows-debug', 'windows-release')]
    [string]$Preset = 'windows-debug'
)

$ErrorActionPreference = 'Stop'
# 어디서 실행하든 저장소 루트(CMakePresets.json 위치)에서 동작하게 한다.
Set-Location (Join-Path $PSScriptRoot '..')
# cl.exe를 PATH에 올린다 (Ninja 프리셋에 필요).
. (Join-Path $PSScriptRoot 'dev_environment.ps1')

# 각 단계가 실패하면 그 종료 코드를 그대로 돌려준다.
cmake --preset $Preset
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

cmake --build --preset $Preset
exit $LASTEXITCODE
