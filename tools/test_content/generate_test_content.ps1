# 이 파일은 무엇을 하나요?
# 시험용 게임 자료를 "그림판처럼" 자동으로 그려 주는 스크립트(명령을 모아 둔 파일)예요.
# 네모, 동그라미 같은 단순한 도형으로 풀밭·나무·캐릭터를 직접 그려요.
# 다른 게임의 그림을 가져다 쓰지 않아서 저작권 걱정이 없어요.
#
# 사용법 (저장소 맨 위 폴더에서):  powershell -NoProfile -File tools\test_content\generate_test_content.ps1
# 만들어지는 파일:
#   content/tilesets/test_tiles.bmp   타일 스티커 판: 32x32 칸, 가로 8개 x 세로 2줄 (1번 = 왼쪽 위)
#   content/sprites/test_hero.bmp     캐릭터 그림 판: 32x48 칸, 가로 4개(걸음) x 세로 4줄(아래·왼쪽·오른쪽·위)
#   content/maps/test_meadow.json     가로 40칸 x 세로 30칸 시험 맵
# 타일 성질(test_tiles.json)과 애니메이션 설명(test_hero.json)은 사람이 직접 고치는 파일이라 만들지 않아요.
# 분홍색(마젠타, 255,0,255)은 "투명"이라는 약속이에요.
# 가장자리를 부드럽게 섞는 기능(안티앨리어싱)을 끄고 그려요. 섞이면 분홍 테두리가 남거든요.

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing

$root = Resolve-Path (Join-Path $PSScriptRoot '..\..')
$content = Join-Path $root 'content'
$transparent = [System.Drawing.Color]::FromArgb(255, 0, 255)

# 새 도화지를 만들어요. 돌려주는 것: (그림, 붓)
function New-Canvas([int]$width, [int]$height) {
    # 24비트 BMP 그림: 게임(SDL_LoadBMP)이 바로 읽을 수 있어요. 처음엔 전체를 투명색(분홍)으로 칠해요.
    $bmp = New-Object System.Drawing.Bitmap($width, $height, [System.Drawing.Imaging.PixelFormat]::Format24bppRgb)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::None
    $g.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::Half
    $g.Clear($transparent)
    return @($bmp, $g)
}

# 빨강·초록·파랑(0~255)을 섞은 색 물감을 만들어요.
function Brush([int]$r, [int]$gr, [int]$b) {
    return New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb($r, $gr, $b))
}

# 그림을 BMP 파일로 저장해요. 폴더가 없으면 먼저 만들어요.
function Save-Bmp($bmp, [string]$path) {
    New-Item -ItemType Directory -Force (Split-Path $path) | Out-Null
    $bmp.Save($path, [System.Drawing.Imaging.ImageFormat]::Bmp)
    Write-Host "wrote $path"
}

# ---------------------------------------------------------------- 타일셋 ----
# 맵 바닥에 깔 타일 스티커 판을 그려요.
$tile = 32
$canvas = New-Canvas (8 * $tile) (2 * $tile)
$bmp = $canvas[0]; $g = $canvas[1]
# 주사위(무작위 숫자)인데, 시작 번호(시드)를 정해 두면 매번 똑같은 순서로 나와요.
# 그래서 스크립트를 다시 돌려도 똑같은 그림이 나와요.
$rng = New-Object System.Random 7

# 타일 번호(1부터)가 스티커 판의 어디(왼쪽 위 픽셀)에 있는지 계산해요.
function TileOrigin([int]$id) {
    $index = $id - 1
    return @((($index % 8) * $tile), ([math]::Floor($index / 8) * $tile))
}

# 바닥 타일 그리기: 바탕색을 칠하고, 조금 다른 색 점을 여기저기 찍어요.
function Fill-Ground([int]$id, $base, $speck, [int]$count) {
    $o = TileOrigin $id
    $g.FillRectangle($base, $o[0], $o[1], $tile, $tile)
    for ($i = 0; $i -lt $count; $i++) {
        $g.FillRectangle($speck, $o[0] + $rng.Next(0, $tile - 2), $o[1] + $rng.Next(0, $tile - 2), 2, 2)
    }
}

Fill-Ground 1 (Brush 92 168 72) (Brush 70 140 56) 18      # 1 풀
Fill-Ground 2 (Brush 78 150 62) (Brush 58 120 48) 26      # 2 짙은 풀
Fill-Ground 3 (Brush 168 128 82) (Brush 140 104 64) 22    # 3 흙길
Fill-Ground 10 (Brush 222 204 150) (Brush 196 178 124) 20 # 10 모래

# 4 물: 파란 바탕에 물결선
$o = TileOrigin 4
$g.FillRectangle((Brush 52 108 196), $o[0], $o[1], $tile, $tile)
foreach ($y in 6, 16, 26) {
    $g.FillRectangle((Brush 120 168 232), $o[0] + 4, $o[1] + $y, 10, 2)
    $g.FillRectangle((Brush 120 168 232), $o[0] + 18, $o[1] + $y + 4, 10, 2)
}

# 5 돌담: 회색 벽돌. 줄마다 벽돌을 반 칸씩 엇갈리게 쌓아요 (진짜 벽돌담처럼).
$o = TileOrigin 5
$g.FillRectangle((Brush 96 96 104), $o[0], $o[1], $tile, $tile)
for ($row = 0; $row -lt 4; $row++) {
    $shift = ($row % 2) * 8
    for ($col = -1; $col -lt 2; $col++) {
        $x = $o[0] + $col * 16 + $shift + 1
        $w = 14
        if ($x -lt $o[0]) { $w -= ($o[0] - $x); $x = $o[0] }
        if ($x + $w -gt $o[0] + $tile) { $w = $o[0] + $tile - $x }
        if ($w -gt 0) { $g.FillRectangle((Brush 140 140 150), $x, $o[1] + $row * 8 + 1, $w, 6) }
    }
}

# 6 나무 밑동 (낮은 물건): 바탕은 투명, 가운데에 갈색 줄기. 못 지나가는 칸이에요.
$o = TileOrigin 6
$g.FillRectangle((Brush 104 70 40), $o[0] + 11, $o[1], 10, 28)
$g.FillRectangle((Brush 80 52 30), $o[0] + 11, $o[1], 3, 28)
$g.FillRectangle((Brush 80 52 30), $o[0] + 7, $o[1] + 26, 18, 4)

# 7 나뭇잎 (높은 물건): 둥근 잎. 캐릭터보다 위에 그려져서, 나무 뒤로 가면 캐릭터가 가려져요.
$o = TileOrigin 7
$g.FillEllipse((Brush 36 104 44), $o[0], $o[1], 32, 32)
$g.FillEllipse((Brush 52 132 56), $o[0] + 4, $o[1] + 3, 22, 20)
$g.FillEllipse((Brush 76 160 72), $o[0] + 9, $o[1] + 6, 9, 8)

# 8 바위 (낮은 물건, 못 지나감)
$o = TileOrigin 8
$g.FillEllipse((Brush 104 104 112), $o[0] + 4, $o[1] + 10, 24, 20)
$g.FillEllipse((Brush 150 150 160), $o[0] + 9, $o[1] + 13, 10, 7)

# 9 꽃 (바닥 장식): 투명 바탕에 작은 꽃 몇 송이
$o = TileOrigin 9
$petals = @((Brush 236 88 112), (Brush 248 216 72), (Brush 236 236 244))
foreach ($p in @(@(6, 8), @(20, 5), @(12, 20), @(24, 22))) {
    $color = $petals[$rng.Next(0, $petals.Count)]
    $g.FillRectangle($color, $o[0] + $p[0], $o[1] + $p[1], 4, 4)
    $g.FillRectangle((Brush 70 120 40), $o[0] + $p[0] + 1, $o[1] + $p[1] + 4, 2, 3)
}

Save-Bmp $bmp (Join-Path $content 'tilesets\test_tiles.bmp')
$g.Dispose(); $bmp.Dispose()

# ----------------------------------------------------- 캐릭터 스프라이트 ----
# 캐릭터 그림 판을 그려요. 표처럼 생겼어요.
# 줄(행): 0 아래 보기, 1 왼쪽 보기, 2 오른쪽 보기, 3 위 보기(뒷모습)
# 칸(열): 0 서기, 1 왼발 앞으로, 2 서기, 3 오른발 앞으로
# 이 칸들을 차례로 넘기면 걷는 것처럼 보여요.
$fw = 32; $fh = 48
$canvas = New-Canvas (4 * $fw) (4 * $fh)
$bmp = $canvas[0]; $g = $canvas[1]

$skin = Brush 240 200 160
$hair = Brush 96 60 36
$tunic = Brush 60 96 180
$tunicDark = Brush 44 72 140
$pants = Brush 72 64 56
$boots = Brush 48 36 28
$eye = Brush 24 24 32

for ($row = 0; $row -lt 4; $row++) {
    for ($col = 0; $col -lt 4; $col++) {
        $x = $col * $fw; $y = $row * $fh
        # 걸음: 1번 칸은 왼발, 3번 칸은 오른발을 앞으로. 걸을 때 몸이 1픽셀 들썩여요.
        $leftLeg = 0; $rightLeg = 0; $bob = 0
        if ($col -eq 1) { $leftLeg = 2; $rightLeg = -2; $bob = -1 }
        if ($col -eq 3) { $leftLeg = -2; $rightLeg = 2; $bob = -1 }

        # 다리와 신발. 발바닥이 y=46에 오게 그려요 (설명서의 "발 위치" origin과 같은 곳).
        $g.FillRectangle($pants, $x + 11, $y + 36 + $bob, 4, 8 + $leftLeg)
        $g.FillRectangle($pants, $x + 17, $y + 36 + $bob, 4, 8 + $rightLeg)
        $g.FillRectangle($boots, $x + 10, $y + 43 + $bob + $leftLeg, 6, 3)
        $g.FillRectangle($boots, $x + 16, $y + 43 + $bob + $rightLeg, 6, 3)

        # 몸통과 팔
        $g.FillRectangle($tunic, $x + 9, $y + 22 + $bob, 14, 15)
        $g.FillRectangle($tunicDark, $x + 9, $y + 34 + $bob, 14, 3)
        if ($row -eq 1 -or $row -eq 2) {
            # 옆모습: 팔 하나가 앞뒤로 흔들린다
            $g.FillRectangle($skin, $x + 14 + $leftLeg, $y + 25 + $bob, 4, 10)
        } else {
            $g.FillRectangle($skin, $x + 6, $y + 24 + $bob - $leftLeg, 3, 10)
            $g.FillRectangle($skin, $x + 23, $y + 24 + $bob - $rightLeg, 3, 10)
        }

        # 머리
        $g.FillEllipse($skin, $x + 8, $y + 6 + $bob, 16, 17)
        switch ($row) {
            0 { # 아래(정면): 앞머리와 두 눈
                $g.FillRectangle($hair, $x + 8, $y + 6 + $bob, 16, 5)
                $g.FillRectangle($eye, $x + 12, $y + 14 + $bob, 2, 3)
                $g.FillRectangle($eye, $x + 18, $y + 14 + $bob, 2, 3)
            }
            1 { # 왼쪽: 뒤통수 머리카락은 오른쪽, 눈은 왼쪽
                $g.FillRectangle($hair, $x + 8, $y + 6 + $bob, 16, 5)
                $g.FillRectangle($hair, $x + 17, $y + 6 + $bob, 7, 12)
                $g.FillRectangle($eye, $x + 10, $y + 14 + $bob, 2, 3)
            }
            2 { # 오른쪽: 왼쪽의 좌우 반전
                $g.FillRectangle($hair, $x + 8, $y + 6 + $bob, 16, 5)
                $g.FillRectangle($hair, $x + 8, $y + 6 + $bob, 7, 12)
                $g.FillRectangle($eye, $x + 20, $y + 14 + $bob, 2, 3)
            }
            3 { # 위(뒷모습): 머리카락이 머리 전체를 덮는다
                $g.FillEllipse($hair, $x + 8, $y + 6 + $bob, 16, 15)
            }
        }
    }
}

Save-Bmp $bmp (Join-Path $content 'sprites\test_hero.bmp')
$g.Dispose(); $bmp.Dispose()

# --------------------------------------------------------------- 시험 맵 ----
# 가로 40칸 x 세로 30칸 맵을 만들어요. 투명 필름 네 장을 겹친다고 생각하면 돼요.
#   ground(바닥), decor(꽃 장식), objects(나무 밑동·바위), canopy(나뭇잎)
# 각 필름은 칸마다 타일 번호를 적은 숫자 목록이에요. 0은 "아무것도 없음".
$W = 40; $H = 30
$ground = New-Object 'int[]' ($W * $H)
$decor = New-Object 'int[]' ($W * $H)
$objects = New-Object 'int[]' ($W * $H)
$canopy = New-Object 'int[]' ($W * $H)
$rng = New-Object System.Random 42

# (x, y) 칸이 숫자 목록의 몇 번째인지: 위의 줄들(y * 가로칸수)을 건너뛰고 x번째
function Idx([int]$x, [int]$y) { return $y * $W + $x }

# 바닥: 전부 풀로 깔고, 15% 정도는 짙은 풀로 얼룩지게
for ($y = 0; $y -lt $H; $y++) {
    for ($x = 0; $x -lt $W; $x++) {
        $ground[(Idx $x $y)] = if ($rng.NextDouble() -lt 0.15) { 2 } else { 1 }
    }
}
# 가로·세로 흙길
for ($x = 2; $x -le 37; $x++) { $ground[(Idx $x 12)] = 3 }
for ($y = 3; $y -le 27; $y++) { $ground[(Idx 20 $y)] = 3 }
# 모래로 둘러싼 연못
for ($y = 17; $y -le 25; $y++) { for ($x = 25; $x -le 34; $x++) { $ground[(Idx $x $y)] = 10 } }
for ($y = 18; $y -le 24; $y++) { for ($x = 26; $x -le 33; $x++) { $ground[(Idx $x $y)] = 4 } }
# 맵 가장자리를 돌담으로 둘러서 캐릭터가 맵 밖으로 못 나가게 해요.
for ($x = 0; $x -lt $W; $x++) { $ground[(Idx $x 0)] = 5; $ground[(Idx $x ($H - 1))] = 5 }
for ($y = 0; $y -lt $H; $y++) { $ground[(Idx 0 $y)] = 5; $ground[(Idx ($W - 1) $y)] = 5 }

# 나무: (x, y)에 밑동을 놓고, 바로 위 칸 (x, y-1)에 나뭇잎을 놓아요.
$trees = @(@(5,5), @(9,4), @(14,7), @(24,5), @(30,8), @(35,5), @(6,20), @(11,24),
           @(16,18), @(23,22), @(36,15), @(34,27), @(4,27), @(8,15), @(13,4))
foreach ($t in $trees) {
    $objects[(Idx $t[0] $t[1])] = 6
    $canopy[(Idx $t[0] ($t[1] - 1))] = 7
}
# 바위
foreach ($r in @(@(12,15), @(28,14), @(17,26), @(8,9), @(31,3))) { $objects[(Idx $r[0] $r[1])] = 8 }

# 꽃 장식: 풀 위이면서 나무·바위가 없는 칸에, 6% 정도 확률로 뿌려요.
for ($y = 1; $y -lt $H - 1; $y++) {
    for ($x = 1; $x -lt $W - 1; $x++) {
        $i = Idx $x $y
        if ($ground[$i] -le 2 -and $objects[$i] -eq 0 -and $rng.NextDouble() -lt 0.06) { $decor[$i] = 9 }
    }
}

# 필름 한 장을 JSON 글자로 바꿔요. 맵 한 줄을 파일 한 줄에 써서 사람이 읽기 쉽게 해요.
function Format-Layer([string]$name, [string]$renderLayer, [int[]]$tiles) {
    $rows = for ($y = 0; $y -lt $H; $y++) {
        '        ' + (($tiles[($y * $W)..($y * $W + $W - 1)]) -join ', ')
    }
    return "    {`n      `"name`": `"$name`",`n      `"render_layer`": `"$renderLayer`",`n      `"tiles`": [`n" +
           ($rows -join ",`n") + "`n      ]`n    }"
}

$layers = @(
    (Format-Layer 'ground' 'ground' $ground),
    (Format-Layer 'decor' 'ground_decor' $decor),
    (Format-Layer 'objects' 'low_objects' $objects),
    (Format-Layer 'canopy' 'high_objects' $canopy)
) -join ",`n"

$json = @"
{
  "schema_version": 1,
  "map_id": "test_meadow",
  "display_name": "Test Meadow",
  "tileset": "tilesets/test_tiles.json",
  "tile_size": 32,
  "width": $W,
  "height": $H,
  "spawn_points": [
    { "id": "default", "x": 6, "y": 12 }
  ],
  "exits": [],
  "layers": [
$layers
  ]
}
"@

$mapPath = Join-Path $content 'maps\test_meadow.json'
New-Item -ItemType Directory -Force (Split-Path $mapPath) | Out-Null
# 맵 파일 저장: 글자 방식은 UTF-8(BOM 없음), 줄바꿈은 LF로 통일해요.
# (그래야 어느 컴퓨터에서 봐도 똑같고, 바뀐 곳 비교도 깔끔해요.)
[System.IO.File]::WriteAllText($mapPath, ($json -replace "`r`n", "`n") + "`n", (New-Object System.Text.UTF8Encoding($false)))
Write-Host "wrote $mapPath"
