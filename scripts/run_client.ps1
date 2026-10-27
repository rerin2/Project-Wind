# 게임 클라이언트를 실행한다. 나머지 인자는 클라이언트에 그대로 전달된다. 예:
#   .\scripts\run_client.ps1 --width 1600 --height 900

# 위치 기반 바인딩을 끈다. 켜 두면 "--width" 같은 첫 인자가 -Preset에 들어간다.
[CmdletBinding(PositionalBinding = $false)]
param(
    [ValidateSet('windows-debug', 'windows-release')]
    [string]$Preset = 'windows-debug',
    # -Preset 외의 모든 인자를 모아 클라이언트 실행 파일에 넘긴다.
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$ClientArgs
)

$ErrorActionPreference = 'Stop'
$exe = Join-Path $PSScriptRoot "..\build\$Preset\bin\wind_client.exe"
if (-not (Test-Path $exe)) {
    throw "Client not built: $exe. Run .\scripts\build.ps1 -Preset $Preset first."
}

# 클라이언트의 종료 코드를 그대로 돌려준다.
& $exe @ClientArgs
exit $LASTEXITCODE
