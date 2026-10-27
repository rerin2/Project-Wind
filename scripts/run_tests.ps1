# 이미 빌드된 프리셋의 테스트를 모두 실행한다 (먼저 build.ps1 실행 필요).
# 사용법: .\scripts\run_tests.ps1 [-Preset windows-debug|windows-release]

param(
    [ValidateSet('windows-debug', 'windows-release')]
    [string]$Preset = 'windows-debug'
)

$ErrorActionPreference = 'Stop'
Set-Location (Join-Path $PSScriptRoot '..')  # 저장소 루트로 이동

# 실패한 테스트가 있으면 ctest가 0이 아닌 종료 코드를 돌려준다.
ctest --preset $Preset
exit $LASTEXITCODE
