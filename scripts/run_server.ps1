# 게임 서버를 실행한다. 나머지 인자는 서버에 그대로 전달된다. 예:
#   .\scripts\run_server.ps1 --log-level debug

# 위치 기반 바인딩을 끈다. 켜 두면 "--max-ticks" 같은 첫 인자가 -Preset에 들어간다.
[CmdletBinding(PositionalBinding = $false)]
param(
    [ValidateSet('windows-debug', 'windows-release')]
    [string]$Preset = 'windows-debug',
    # -Preset 외의 모든 인자를 모아 서버 실행 파일에 넘긴다.
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$ServerArgs
)

$ErrorActionPreference = 'Stop'
$exe = Join-Path $PSScriptRoot "..\build\$Preset\bin\wind_server.exe"
if (-not (Test-Path $exe)) {
    throw "Server not built: $exe. Run .\scripts\build.ps1 -Preset $Preset first."
}

# 서버의 종료 코드를 그대로 돌려준다.
& $exe @ServerArgs
exit $LASTEXITCODE
