# 이 파일은 무엇을 하나요?
# 시험용 게임 자료를 "그림판처럼" 자동으로 그려 주는 스크립트(명령을 모아 둔 파일)예요.
# 네모, 동그라미 같은 단순한 도형으로 풀밭·나무·캐릭터를 직접 그려요.
# 다른 게임의 그림을 가져다 쓰지 않아서 저작권 걱정이 없어요.
#
# 사용법 (저장소 맨 위 폴더에서):  powershell -NoProfile -File tools\test_content\generate_test_content.ps1
# 만들어지는 파일:
#   content/tilesets/test_tiles.bmp   타일 스티커 판: 32x32 칸, 가로 8개 x 세로 3줄 (1번 = 왼쪽 위)
#   content/ui/ui_frames.bmp          UI 창·버튼 9조각 테두리 (72x24)
#   content/props/training_dummy.bmp  훈련용 허수아비 (96x48: 서 있음 / 쓰러짐, 칸 48x48)
#   content/monsters/slime_green.bmp  초록 슬라임 (48x24: 보통 / 공격 예고, 칸 24x24)
#   content/fx/slash_arc.bmp          칼자국 반달 (32x32, 흰색)
#   content/sprites/test_hero.bmp     주인공(고양이 귀 은발 소녀) 그림 판: 32x48 칸, 가로 4개(걸음) x 세로 5줄(아래·왼쪽·오른쪽·위 + 13b 쿠션 동작)
#   content/props/catnip_cushion.bmp  캣닢 쿠션 (128x32: 시든 / 피어난 / z / 하트, 칸 32x32, 13b)
#   content/monsters/bat.bmp          숲 박쥐 (48x24: 보통 / 공격 예고, 13c)
#   content/monsters/mushroom.bmp     가시 버섯 (64x32: 보통 / 공격 예고 가시, 13c)
#   content/monsters/frog.bmp         침 뱉는 개구리 (48x24: 보통 / 공격 예고 볼 부풀림, 13c)
#   content/items/item_icons.bmp      아이템 아이콘 (160x16, 칸 16x16, 도감 icon 번호 순서, 14b)
#   content/maps/test_meadow.json     가로 40칸 x 세로 30칸 시험 맵 (왼쪽 끝 동굴 입구, 오른쪽 끝 숲길)
#   content/maps/test_cave.json       가로 30칸 x 세로 20칸 조명 시험 동굴
#   content/maps/test_forest.json     가로 36칸 x 세로 24칸 시험 숲 (키 큰 풀·나뭇잎이 바람에 흔들려요)
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
$canvas = New-Canvas (8 * $tile) (3 * $tile)
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

# 4 물: 파란 바탕에 물결선. 14, 15번은 물결이 조금씩 옮겨 가고 반짝이가 생긴 그림이에요.
# 타일셋 설명서(animation)가 4 -> 14 -> 15 -> 14 순서로 바꿔 보여 줘서 물이 출렁이며 반짝여요.
# shift: 물결을 오른쪽으로 몇 픽셀 옮길지, sparkles: 반짝이 자리 (x, y) 목록
function Draw-Water([int]$id, [int]$shift, $sparkles) {
    $o = TileOrigin $id
    $g.FillRectangle((Brush 52 108 196), $o[0], $o[1], $tile, $tile)
    foreach ($y in 6, 16, 26) {
        $g.FillRectangle((Brush 120 168 232), $o[0] + 4 + $shift, $o[1] + $y, 10, 2)
        $g.FillRectangle((Brush 120 168 232), $o[0] + 18 - $shift, $o[1] + $y + 4, 10, 2)
    }
    foreach ($s in $sparkles) {
        $g.FillRectangle((Brush 228 242 255), $o[0] + $s[0], $o[1] + $s[1], 2, 2)
        $g.FillRectangle((Brush 170 206 246), $o[0] + $s[0] - 1, $o[1] + $s[1] + 2, 4, 1)
    }
}
Draw-Water 4 0 @()
Draw-Water 14 2 @(@(9, 12))
Draw-Water 15 4 @(@(23, 2), @(5, 22))

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

# 9 꽃 (바닥 장식): 투명 바탕에 작은 꽃 몇 송이.
# 16번은 같은 꽃이 바람에 꽃송이만 1픽셀 기운 그림이에요. 9 <-> 16을 번갈아 보여 주면 살랑거려요.
$petals = @((Brush 236 88 112), (Brush 248 216 72), (Brush 236 236 244))
$flowerSpots = @(@(6, 8), @(20, 5), @(12, 20), @(24, 22))
$flowerColors = foreach ($p in $flowerSpots) { $petals[$rng.Next(0, $petals.Count)] }
function Draw-Flowers([int]$id, [int]$lean) {
    $o = TileOrigin $id
    for ($i = 0; $i -lt $flowerSpots.Count; $i++) {
        $p = $flowerSpots[$i]
        $g.FillRectangle($flowerColors[$i], $o[0] + $p[0] + $lean, $o[1] + $p[1], 4, 4)
        $g.FillRectangle((Brush 70 120 40), $o[0] + $p[0] + 1, $o[1] + $p[1] + 4, 2, 3)
    }
}
Draw-Flowers 9 0
Draw-Flowers 16 1

# 11 횃불 (낮은 물건, 못 지나감): 나무 기둥 + 쇠 받침 + 불꽃. 빛은 맵의 "lights"가 따로 비춰요.
$o = TileOrigin 11
$g.FillRectangle((Brush 92 60 40), $o[0] + 14, $o[1] + 12, 4, 18)    # 나무 기둥
$g.FillRectangle((Brush 70 44 30), $o[0] + 14, $o[1] + 12, 1, 18)    # 기둥 그늘
$g.FillRectangle((Brush 60 60 70), $o[0] + 11, $o[1] + 10, 10, 3)    # 쇠 받침
$g.FillRectangle((Brush 60 60 70), $o[0] + 12, $o[1] + 28, 8, 2)     # 바닥 받침
$g.FillEllipse((Brush 232 106 58), $o[0] + 11, $o[1] + 1, 10, 11)    # 불꽃 바깥 (ember)
$g.FillEllipse((Brush 242 181 68), $o[0] + 13, $o[1] + 4, 6, 7)      # 불꽃 안쪽 (gold)
$g.FillRectangle((Brush 255 240 168), $o[0] + 15, $o[1] + 7, 2, 3)   # 불꽃 심지 (sun)

# 12 동굴 입구 (지나갈 수 있음): 돌담에 어두운 아치형 구멍. 이 칸을 밟으면 다른 맵으로 가요.
$o = TileOrigin 12
$g.FillRectangle((Brush 96 96 104), $o[0], $o[1], $tile, $tile)                 # 돌담 바탕
$g.FillRectangle((Brush 140 140 150), $o[0] + 1, $o[1] + 1, 6, 6)                # 벽돌 몇 장
$g.FillRectangle((Brush 140 140 150), $o[0] + 25, $o[1] + 1, 6, 6)
$g.FillRectangle((Brush 140 140 150), $o[0] + 1, $o[1] + 25, 4, 6)
$g.FillRectangle((Brush 140 140 150), $o[0] + 27, $o[1] + 25, 4, 6)
$g.FillEllipse((Brush 36 30 48), $o[0] + 5, $o[1] + 4, 22, 20)                   # 아치 윗부분
$g.FillRectangle((Brush 36 30 48), $o[0] + 5, $o[1] + 14, 22, 18)                # 구멍 아래쪽
$g.FillEllipse((Brush 20 16 28), $o[0] + 9, $o[1] + 9, 14, 14)                   # 안쪽 더 깊은 어둠
$g.FillRectangle((Brush 20 16 28), $o[0] + 9, $o[1] + 16, 14, 16)

# 13 가로등 (낮은 물건, 못 지나감): 나무 기둥 위에 유리 등. 밤이 되면 맵의 빛(night_only)이 켜져요.
$o = TileOrigin 13
$g.FillRectangle((Brush 70 48 36), $o[0] + 15, $o[1] + 10, 3, 20)     # 기둥
$g.FillRectangle((Brush 50 34 26), $o[0] + 15, $o[1] + 10, 1, 20)     # 기둥 그늘
$g.FillRectangle((Brush 60 60 70), $o[0] + 11, $o[1] + 28, 11, 3)     # 받침
$g.FillRectangle((Brush 60 60 70), $o[0] + 11, $o[1] + 1, 11, 2)      # 등 지붕
$g.FillRectangle((Brush 60 60 70), $o[0] + 11, $o[1] + 9, 11, 2)      # 등 바닥
$g.FillRectangle((Brush 242 214 150), $o[0] + 12, $o[1] + 3, 9, 6)    # 유리 (불빛)
$g.FillRectangle((Brush 255 244 200), $o[0] + 15, $o[1] + 4, 3, 4)    # 심지

# ---- 숲 타일 (17~21) ----
# 17 키 큰 풀 (바닥 장식, 지나갈 수 있음): 뾰족한 풀잎 다발. 바람에 흔들려요(sway).
$o = TileOrigin 17
foreach ($tuft in @(@(4, 18), @(16, 8), @(22, 22), @(10, 26))) {
    foreach ($blade in @(@(0, 7, 92, 158, 70), @(2, 10, 70, 136, 56), @(4, 6, 110, 176, 84))) {
        $b = Brush $blade[2] $blade[3] $blade[4]
        $g.FillRectangle($b, $o[0] + $tuft[0] + $blade[0], $o[1] + $tuft[1] - $blade[1], 2, $blade[1])
    }
}
# 18 덤불 (낮은 물건, 못 지나감): 숲 가장자리를 막는 둥근 덤불
$o = TileOrigin 18
$g.FillEllipse((Brush 30 82 40), $o[0] + 1, $o[1] + 6, 30, 25)
$g.FillEllipse((Brush 44 108 50), $o[0] + 4, $o[1] + 8, 20, 15)
$g.FillEllipse((Brush 64 134 62), $o[0] + 8, $o[1] + 10, 8, 6)
$g.FillRectangle((Brush 196 60 72), $o[0] + 20, $o[1] + 18, 2, 2)  # 빨간 열매
$g.FillRectangle((Brush 196 60 72), $o[0] + 11, $o[1] + 23, 2, 2)
# 19 숲 바닥 (지나갈 수 있음): 짙은 흙빛 풀에 낙엽 점
Fill-Ground 19 (Brush 66 112 58) (Brush 54 92 46) 22
$o = TileOrigin 19
foreach ($i in 1..5) {
    $leaf = if ($i % 2) { Brush 150 112 54 } else { Brush 120 132 52 }
    $g.FillRectangle($leaf, $o[0] + $rng.Next(1, 29), $o[1] + $rng.Next(1, 29), 3, 2)
}
# 20 버섯 (바닥 장식): 빨간 갓에 흰 점
$o = TileOrigin 20
foreach ($m in @(@(8, 14), @(19, 20))) {
    $g.FillRectangle((Brush 232 224 204), $o[0] + $m[0] + 2, $o[1] + $m[1] + 3, 3, 4)
    $g.FillEllipse((Brush 200 56 52), $o[0] + $m[0], $o[1] + $m[1], 7, 5)
    $g.FillRectangle((Brush 250 240 230), $o[0] + $m[0] + 2, $o[1] + $m[1] + 1, 1, 1)
}
# 21 짙은 나뭇잎 (높은 물건): 숲 나무용. 7번보다 어둡고 큼직해요. 바람에 흔들려요.
$o = TileOrigin 21
$g.FillEllipse((Brush 24 72 40), $o[0], $o[1], 32, 32)
$g.FillEllipse((Brush 36 96 48), $o[0] + 3, $o[1] + 2, 24, 22)
$g.FillEllipse((Brush 56 124 60), $o[0] + 8, $o[1] + 5, 10, 9)

Save-Bmp $bmp (Join-Path $content 'tilesets\test_tiles.bmp')
$g.Dispose(); $bmp.Dispose()

# ------------------------------------------------------------- UI 테두리 ----
# 9조각 창·버튼 테두리 그림 판 (72x24). 위치와 테두리 두께는 content/ui/theme.json에 적어요.
#   창(0,0,24,24, 테두리 8) / 버튼(24,0,24,16, 테두리 5) / 고른 버튼(48,0,24,16, 테두리 5)
# 네 모서리는 1픽셀씩 깎아서(분홍 = 투명) 살짝 둥글게 보여요.
$canvas = New-Canvas 72 24
$bmp = $canvas[0]; $g = $canvas[1]

# 둥근 네모 테두리 하나: 바깥 검은 선 -> 테두리 색 -> 안쪽 선 -> 바탕
function Draw-Frame([int]$x, [int]$y, [int]$w, [int]$h, $edge, $inner, $fill, $highlight) {
    $outline = Brush 16 19 28
    $g.FillRectangle($outline, $x + 1, $y, $w - 2, $h)
    $g.FillRectangle($outline, $x, $y + 1, $w, $h - 2)
    $g.FillRectangle($edge, $x + 1, $y + 1, $w - 2, $h - 2)
    $g.FillRectangle($inner, $x + 2, $y + 2, $w - 4, $h - 4)
    $g.FillRectangle($fill, $x + 3, $y + 3, $w - 6, $h - 6)
    $g.FillRectangle($highlight, $x + 3, $y + 3, $w - 6, 1)   # 위쪽 빛 받는 줄
}
# 창: 짙은 남색 바탕에 은색 테두리, 네 모서리에 금색 점
Draw-Frame 0 0 24 24 (Brush 150 160 184) (Brush 58 66 92) (Brush 28 32 50) (Brush 40 46 68)
foreach ($c in @(@(3, 3), @(19, 3), @(3, 19), @(19, 19))) {
    $g.FillRectangle((Brush 232 200 120), $c[0], $c[1], 2, 2)
}
# 버튼: 보통은 차분한 남색, 고른 버튼은 밝은 바탕에 금색 테두리
Draw-Frame 24 0 24 16 (Brush 96 106 132) (Brush 52 58 84) (Brush 44 50 74) (Brush 70 78 108)
Draw-Frame 48 0 24 16 (Brush 232 200 120) (Brush 120 100 64) (Brush 64 72 106) (Brush 96 104 140)

Save-Bmp $bmp (Join-Path $content 'ui\ui_frames.bmp')
$g.Dispose(); $bmp.Dispose()

# ------------------------------------------------------- 훈련용 허수아비 ----
# 허수아비 그림 판 (96x48, 칸 48x48): 왼쪽 칸 = 서 있음, 오른쪽 칸 = 쓰러짐(옆으로 기울어짐).
# 발 위치는 칸의 (24, 44). content/props/training_dummy.json의 "origin"과 같아요.
$canvas = New-Canvas 96 48
$bmp = $canvas[0]; $g = $canvas[1]

# 허수아비 하나를 그려요. ox: 칸 왼쪽 위치, angle: 발을 중심으로 기울이는 각도(도)
function Draw-Dummy([int]$ox, [float]$angle) {
    $g.TranslateTransform($ox + 24, 44)    # 발 위치를 (0, 0)으로 옮겨요
    $g.RotateTransform($angle)             # 발을 중심으로 기울여요
    $g.FillRectangle((Brush 80 52 30), -6, -2, 12, 3)        # 바닥 받침
    $g.FillRectangle((Brush 104 70 40), -2, -30, 4, 30)      # 나무 기둥
    $g.FillRectangle((Brush 80 52 30), -2, -30, 1, 30)       # 기둥 그늘
    $g.FillRectangle((Brush 104 70 40), -12, -23, 24, 3)     # 팔 막대
    $g.FillEllipse((Brush 196 168 104), -8, -28, 16, 21)     # 짚 몸통
    $g.FillRectangle((Brush 168 140 80), -6, -24, 1, 14)     # 짚 결
    $g.FillRectangle((Brush 168 140 80), 5, -23, 1, 12)
    $g.FillRectangle((Brush 120 84 48), -8, -12, 16, 2)      # 허리 끈
    $g.FillEllipse((Brush 200 60 52), -4, -24, 8, 8)         # 과녁 바깥 (빨강)
    $g.FillEllipse((Brush 240 230 210), -2, -22, 4, 4)       # 과녁 가운데 (흰색)
    $g.FillRectangle((Brush 200 60 52), -1, -21, 2, 2)       # 과녁 점
    $g.FillEllipse((Brush 210 184 124), -6, -41, 12, 13)     # 머리 (자루)
    $g.FillRectangle((Brush 90 60 40), -4, -37, 2, 2)        # 꿰맨 눈 (왼쪽)
    $g.FillRectangle((Brush 90 60 40), 2, -37, 2, 2)         # 꿰맨 눈 (오른쪽)
    $g.FillRectangle((Brush 90 60 40), -2, -33, 4, 1)        # 꿰맨 입
    $g.FillRectangle((Brush 232 204 112), -3, -43, 2, 3)     # 머리 위 짚 삐침
    $g.FillRectangle((Brush 232 204 112), 1, -44, 2, 4)
    $g.ResetTransform()                    # 다음 그림을 위해 기울기를 되돌려요
}
Draw-Dummy 0 0      # 서 있음
Draw-Dummy 48 22    # 쓰러짐: 22도 기울어짐 (머리가 칸 밖으로 나가지 않을 만큼)
Save-Bmp $bmp (Join-Path $content 'props\training_dummy.bmp')
$g.Dispose(); $bmp.Dispose()

# ------------------------------------------------------------ 초록 슬라임 ----
# 슬라임 그림 판 (48x24, 칸 24x24): 왼쪽 칸 = 보통, 오른쪽 칸 = 공격 예고(납작 웅크리고 눈을 찡그림, 11b).
# 발 위치는 칸의 (12, 21). content/monsters/slime_green.json의 "origin"과 같아요.
# 숨쉬기·통통 뛰기는 게임이 이 그림을 늘이고 줄여서 만들어요 (monster_pose).
$canvas = New-Canvas 48 24
$bmp = $canvas[0]; $g = $canvas[1]
$clear = New-Object System.Drawing.SolidBrush($transparent)

# 슬라임 하나를 그려요. ox: 칸 왼쪽 위치, w/h: 몸 크기, angry: 찡그린 눈썹
function Draw-Slime([int]$ox, [int]$w, [int]$h, [bool]$angry) {
    $left = $ox + 12 - [int]($w / 2)         # 몸 왼쪽 끝 (가운데 맞춤)
    $top = 22 - $h                           # 몸 위쪽 끝 (바닥은 늘 22줄)
    $g.FillEllipse((Brush 28 66 40), $left, $top, $w, $h + 4)              # 짙은 테두리
    $g.FillEllipse((Brush 62 140 70), $left + 1, $top + 1, $w - 2, $h + 2)  # 아래쪽 그늘 색
    $g.FillEllipse((Brush 96 186 88), $left + 2, $top + 1, $w - 5, $h - 2)  # 몸통 (밝은 초록)
    $g.FillEllipse((Brush 200 244 170), $left + 4, $top + 2, 5, 3)         # 반짝이는 윗부분
    $g.FillRectangle((Brush 236 255 220), $left + 5, $top + 3, 1, 1)       # 가장 밝은 점
    $g.FillRectangle($clear, $ox, 22, 24, 2)                                # 바닥 아래는 투명으로 잘라 평평하게
    $g.FillRectangle((Brush 28 66 40), $left + 2, 21, $w - 4, 1)           # 바닥 테두리
    $eyeY = 22 - [int]($h / 2)                                             # 눈 높이 (몸 가운데쯤)
    $g.FillRectangle((Brush 24 40 32), $ox + 8, $eyeY, 2, 3)               # 왼쪽 눈
    $g.FillRectangle((Brush 24 40 32), $ox + 14, $eyeY, 2, 3)              # 오른쪽 눈
    $g.FillRectangle((Brush 255 255 255), $ox + 8, $eyeY, 1, 1)            # 눈 반짝
    $g.FillRectangle((Brush 255 255 255), $ox + 14, $eyeY, 1, 1)
    if ($angry) {                                                          # 찡그린 눈썹: 안쪽이 아래로
        $g.FillRectangle((Brush 24 40 32), $ox + 7, $eyeY - 2, 2, 1)
        $g.FillRectangle((Brush 24 40 32), $ox + 9, $eyeY - 1, 1, 1)
        $g.FillRectangle((Brush 24 40 32), $ox + 15, $eyeY - 2, 2, 1)
        $g.FillRectangle((Brush 24 40 32), $ox + 14, $eyeY - 1, 1, 1)
    }
}
Draw-Slime 0 20 15 $false    # 보통: 둥근 물방울
Draw-Slime 24 24 11 $true    # 공격 예고: 넓고 납작하게 웅크림
Save-Bmp $bmp (Join-Path $content 'monsters\slime_green.bmp')
$g.Dispose(); $bmp.Dispose()

# --------------------------------------------------------- 칼자국 (반달) ----
# 휘두를 때 히트박스 자리에 잠깐 보이는 하얀 반달 (32x32). 오른쪽을 볼 때 모양이고, 게임이 방향대로 돌려요.
# 흰색이라 게임에서 색을 곱하고(빛나게 Add 섞기) 투명도를 줄이며 사라지게 해요.
$canvas = New-Canvas 32 32
$bmp = $canvas[0]; $g = $canvas[1]
$g.FillEllipse((Brush 255 255 255), 2, 2, 28, 28)        # 큰 흰 동그라미
$g.FillEllipse((New-Object System.Drawing.SolidBrush($transparent)), -5, 3, 27, 26)  # 투명(분홍)으로 왼쪽을 파내서 반달만 남겨요
Save-Bmp $bmp (Join-Path $content 'fx\slash_arc.bmp')
$g.Dispose(); $bmp.Dispose()

# ----------------------------------------------------- 캐릭터 스프라이트 ----
# 캐릭터 그림 판을 그려요. 표처럼 생겼어요.
# 줄(행): 0 아래 보기, 1 왼쪽 보기, 2 오른쪽 보기, 3 위 보기(뒷모습)
# 칸(열): 0 서기, 1 왼발 앞으로, 2 서기, 3 오른발 앞으로
# 이 칸들을 차례로 넘기면 걷는 것처럼 보여요.
# 4번 줄(13b): 캣닢 쿠션 동작, 모두 정면. 0 앉기, 1 웅크리기(졸음), 2 기지개, 3 앉기(웃는 눈)
$fw = 32; $fh = 48
$canvas = New-Canvas (4 * $fw) (5 * $fh)
$bmp = $canvas[0]; $g = $canvas[1]

# 주인공: 고양이 귀가 달린 은발 긴 생머리 소녀.
# 남색 원피스(흰 테두리, 빨간 리본), 짙은 타이츠, 갈색 부츠, 보라색 눈.
# 색은 docs/VISUALS.md 팔레트의 은빛·남색 계열에서 골랐어요.
$skin      = Brush 250 222 200
$skinShade = Brush 226 184 164
$blush     = Brush 244 166 166
$hair      = Brush 206 212 228   # 은발 기본색
$hairShade = Brush 156 162 190   # 은발 그늘 (뒷머리, 머리카락 결)
$hairLight = Brush 242 244 252   # 은발 반짝임
$earInner  = Brush 238 168 186   # 고양이 귀 안쪽 분홍
$dress     = Brush 60 70 128
$dressDark = Brush 42 48 96
$trim      = Brush 236 232 244   # 흰 테두리
$ribbon    = Brush 208 72 100
$tights    = Brush 52 46 66
$boots     = Brush 98 64 54
$iris      = Brush 122 96 214
$lash      = Brush 44 34 70
$shine     = Brush 255 255 255
$ink       = [System.Drawing.Color]::FromArgb(28, 22, 40)   # 바깥 테두리 (팔레트 0번 ink)

# 여러 점을 이어 삼각형 등을 칠해요. 점은 (x, y) 쌍의 목록이에요.
function Poly($brush, [int[]]$xy) {
    $points = New-Object 'System.Drawing.Point[]' ($xy.Count / 2)
    for ($i = 0; $i -lt $points.Count; $i++) {
        $points[$i] = New-Object System.Drawing.Point($xy[2 * $i], $xy[2 * $i + 1])
    }
    $g.FillPolygon($brush, $points)
}

# 다리·부츠·치마·몸통: 모든 방향이 거의 같아요. 발바닥이 y=46에 오게 그려요 (origin과 같은 곳).
function Draw-Body([int]$x, [int]$y, [int]$b, [int]$leftLeg, [int]$rightLeg, [bool]$front) {
    $g.FillRectangle($tights, $x + 12, $y + 36 + $b, 3, 8 + $leftLeg)
    $g.FillRectangle($tights, $x + 17, $y + 36 + $b, 3, 8 + $rightLeg)
    $g.FillRectangle($boots, $x + 11, $y + 42 + $b + $leftLeg, 5, 4)
    $g.FillRectangle($boots, $x + 16, $y + 42 + $b + $rightLeg, 5, 4)
    # 치마: 아래로 갈수록 넓어지는 사다리꼴, 밑단은 흰 테두리
    Poly $dress @(($x + 10), ($y + 28 + $b), ($x + 22), ($y + 28 + $b), ($x + 24), ($y + 37 + $b), ($x + 8), ($y + 37 + $b))
    $g.FillRectangle($dressDark, $x + 19, $y + 30 + $b, 3, 6)
    $g.FillRectangle($trim, $x + 8, $y + 36 + $b, 17, 1)
    # 윗몸
    $g.FillRectangle($dress, $x + 11, $y + 22 + $b, 10, 7)
    if ($front) {
        $g.FillRectangle($trim, $x + 12, $y + 22 + $b, 8, 1)      # 흰 깃
        $g.FillRectangle($ribbon, $x + 15, $y + 23 + $b, 2, 2)    # 빨간 리본
    }
}

# 고양이 귀 한 쌍 (정면·뒷면용). inner: 안쪽 분홍을 보일지 (뒷모습은 안 보여요)
function Draw-Ears([int]$x, [int]$y, [int]$b, [bool]$inner) {
    Poly $hair @(($x + 8), ($y + 10 + $b), ($x + 10), ($y + 1 + $b), ($x + 15), ($y + 7 + $b))
    Poly $hair @(($x + 17), ($y + 7 + $b), ($x + 22), ($y + 1 + $b), ($x + 24), ($y + 10 + $b))
    if ($inner) {
        Poly $earInner @(($x + 10), ($y + 7 + $b), ($x + 11), ($y + 4 + $b), ($x + 13), ($y + 7 + $b))
        Poly $earInner @(($x + 19), ($y + 7 + $b), ($x + 21), ($y + 4 + $b), ($x + 22), ($y + 7 + $b))
    } else {
        $g.FillRectangle($hairShade, $x + 11, $y + 4 + $b, 1, 4)
        $g.FillRectangle($hairShade, $x + 21, $y + 4 + $b, 1, 4)
    }
}

# 정면 뒷머리: 등 뒤로 허리까지 내려오는 긴 머리 (몸보다 먼저 = 뒤에 그려요). sway: 머리 끝 살랑임
function Draw-FrontBackHair([int]$x, [int]$y, [int]$b, [int]$sway) {
    $g.FillRectangle($hairShade, $x + 7, $y + 12 + $b, 18, 20)
    $g.FillRectangle($hairShade, $x + 8 + $sway, $y + 32 + $b, 4, 2)
    $g.FillRectangle($hairShade, $x + 20 + $sway, $y + 32 + $b, 4, 2)
}

# 정면 얼굴·머리·귀·눈. b: 위아래로 옮기는 양, eyes: 'open'(보통) / 'closed'(감음) / 'happy'(^ ^, 13b)
function Draw-FrontHead([int]$x, [int]$y, [int]$b, [string]$eyes) {
    # 3) 얼굴
    $g.FillEllipse($skin, $x + 9, $y + 8 + $b, 14, 15)
    # 4) 정수리 머리와 앞머리 (삐죽삐죽한 끝)
    $g.FillRectangle($hair, $x + 9, $y + 6 + $b, 14, 6)
    $g.FillRectangle($hair, $x + 8, $y + 8 + $b, 16, 3)
    $g.FillRectangle($hair, $x + 11, $y + 12 + $b, 2, 2)
    $g.FillRectangle($hair, $x + 15, $y + 12 + $b, 2, 1)
    $g.FillRectangle($hair, $x + 19, $y + 12 + $b, 2, 2)
    $g.FillRectangle($hairLight, $x + 12, $y + 7 + $b, 5, 1)
    # 5) 얼굴 옆으로 내려오는 옆머리 (어깨 앞까지, 손은 가리지 않게)
    $g.FillRectangle($hair, $x + 7, $y + 11 + $b, 3, 15)
    $g.FillRectangle($hair, $x + 22, $y + 11 + $b, 3, 15)
    $g.FillRectangle($hairLight, $x + 8, $y + 13 + $b, 1, 9)
    # 6) 고양이 귀
    Draw-Ears $x $y $b $true
    # 7) 눈: 보통은 속눈썹(진한 줄) + 보라색 눈동자 + 반짝이는 점. 그다음 볼 터치, 입
    switch ($eyes) {
        'closed' {  # 감은 눈: 아래로 처진 짧은 줄 (졸려요)
            $g.FillRectangle($lash, $x + 11, $y + 16 + $b, 3, 1)
            $g.FillRectangle($lash, $x + 18, $y + 16 + $b, 3, 1)
        }
        'happy' {   # 웃는 눈: ^ ^ 모양 (가운데가 1픽셀 위)
            $g.FillRectangle($lash, $x + 11, $y + 16 + $b, 1, 1)
            $g.FillRectangle($lash, $x + 12, $y + 15 + $b, 1, 1)
            $g.FillRectangle($lash, $x + 13, $y + 16 + $b, 1, 1)
            $g.FillRectangle($lash, $x + 18, $y + 16 + $b, 1, 1)
            $g.FillRectangle($lash, $x + 19, $y + 15 + $b, 1, 1)
            $g.FillRectangle($lash, $x + 20, $y + 16 + $b, 1, 1)
        }
        default {
            $g.FillRectangle($lash, $x + 11, $y + 14 + $b, 3, 1)
            $g.FillRectangle($lash, $x + 18, $y + 14 + $b, 3, 1)
            $g.FillRectangle($iris, $x + 12, $y + 15 + $b, 2, 3)
            $g.FillRectangle($iris, $x + 18, $y + 15 + $b, 2, 3)
            $g.FillRectangle($shine, $x + 12, $y + 15 + $b, 1, 1)
            $g.FillRectangle($shine, $x + 18, $y + 15 + $b, 1, 1)
        }
    }
    $g.FillRectangle($blush, $x + 10, $y + 19 + $b, 2, 1)
    $g.FillRectangle($blush, $x + 20, $y + 19 + $b, 2, 1)
    $g.FillRectangle($skinShade, $x + 15, $y + 20 + $b, 2, 1)
}

for ($row = 0; $row -lt 4; $row++) {
    if ($row -eq 2) { continue }   # 오른쪽 보기는 아래에서 왼쪽 보기를 거울처럼 뒤집어 만들어요
    for ($col = 0; $col -lt 4; $col++) {
        $x = $col * $fw; $y = $row * $fh
        # 걸음: 1번 칸은 왼발, 3번 칸은 오른발을 앞으로. 걸을 때 몸이 1픽셀 들썩이고
        # 긴 머리 끝이 반대쪽으로 1픽셀 살랑여요.
        $leftLeg = 0; $rightLeg = 0; $b = 0; $sway = 0
        if ($col -eq 1) { $leftLeg = 2; $rightLeg = -2; $b = -1; $sway = 1 }
        if ($col -eq 3) { $leftLeg = -2; $rightLeg = 2; $b = -1; $sway = -1 }

        switch ($row) {
            0 { # 아래(정면)
                Draw-FrontBackHair $x $y $b $sway   # 1) 뒷머리
                Draw-Body $x $y $b $leftLeg $rightLeg $true
                # 2) 팔: 소매(남색) + 손(살색), 걸을 때 앞뒤로 흔들려요
                $g.FillRectangle($dress, $x + 8, $y + 22 + $b, 3, 4)
                $g.FillRectangle($dress, $x + 21, $y + 22 + $b, 3, 4)
                $g.FillRectangle($skin, $x + 8, $y + 26 + $b - $leftLeg, 3, 6)
                $g.FillRectangle($skin, $x + 21, $y + 26 + $b - $rightLeg, 3, 6)
                Draw-FrontHead $x $y $b 'open'      # 3)~7) 얼굴·머리·귀·눈
            }
            1 { # 왼쪽 보기 (옆모습)
                # 1) 뒷머리: 등 뒤(오른쪽)로 길게
                $g.FillRectangle($hairShade, $x + 13, $y + 10 + $b, 11, 22)
                $g.FillRectangle($hairShade, $x + 16 + $sway, $y + 32 + $b, 7, 2)
                Draw-Body $x $y $b $leftLeg $rightLeg $false
                # 2) 팔 하나가 앞뒤로 흔들려요
                $g.FillRectangle($dress, $x + 14, $y + 22 + $b, 4, 4)
                $g.FillRectangle($skin, $x + 14 + $leftLeg, $y + 26 + $b, 3, 6)
                # 3) 얼굴 (왼쪽으로 조금 치우침)
                $g.FillEllipse($skin, $x + 8, $y + 8 + $b, 14, 15)
                # 4) 머리: 정수리, 뒤통수(오른쪽), 앞머리(왼쪽)
                $g.FillRectangle($hair, $x + 10, $y + 6 + $b, 13, 6)
                $g.FillRectangle($hair, $x + 15, $y + 6 + $b, 9, 15)
                $g.FillRectangle($hair, $x + 8, $y + 9 + $b, 5, 4)
                $g.FillRectangle($hairLight, $x + 13, $y + 7 + $b, 5, 1)
                $g.FillRectangle($hairLight, $x + 19, $y + 10 + $b, 1, 8)
                # 5) 고양이 귀: 앞쪽 귀는 분홍 안쪽이 보이고, 뒤쪽 귀는 그늘색
                Poly $hairShade @(($x + 18), ($y + 8 + $b), ($x + 22), ($y + 2 + $b), ($x + 24), ($y + 10 + $b))
                Poly $hair @(($x + 11), ($y + 8 + $b), ($x + 14), ($y + 1 + $b), ($x + 18), ($y + 7 + $b))
                Poly $earInner @(($x + 13), ($y + 6 + $b), ($x + 14), ($y + 3 + $b), ($x + 16), ($y + 6 + $b))
                # 6) 눈과 볼
                $g.FillRectangle($lash, $x + 9, $y + 14 + $b, 3, 1)
                $g.FillRectangle($iris, $x + 10, $y + 15 + $b, 2, 3)
                $g.FillRectangle($shine, $x + 10, $y + 15 + $b, 1, 1)
                $g.FillRectangle($blush, $x + 9, $y + 19 + $b, 2, 1)
            }
            3 { # 위 보기 (뒷모습): 은발이 머리와 등을 허리까지 덮고, 그 아래로 치마가 보여요
                Draw-Body $x $y $b $leftLeg $rightLeg $false
                # 팔은 머리카락 바깥쪽에 살짝 보이게
                $g.FillRectangle($dress, $x + 7, $y + 22 + $b, 3, 4)
                $g.FillRectangle($dress, $x + 22, $y + 22 + $b, 3, 4)
                $g.FillRectangle($skin, $x + 7, $y + 26 + $b - $leftLeg, 3, 6)
                $g.FillRectangle($skin, $x + 22, $y + 26 + $b - $rightLeg, 3, 6)
                $g.FillEllipse($hair, $x + 8, $y + 5 + $b, 16, 16)
                $g.FillRectangle($hair, $x + 9, $y + 12 + $b, 14, 20)
                $g.FillRectangle($hair, $x + 10 + $sway, $y + 32 + $b, 12, 2)
                # 머리카락 결 (세로 그늘 줄)과 반짝임
                $g.FillRectangle($hairShade, $x + 12, $y + 16 + $b, 1, 16)
                $g.FillRectangle($hairShade, $x + 16, $y + 18 + $b, 1, 14)
                $g.FillRectangle($hairShade, $x + 20, $y + 16 + $b, 1, 16)
                $g.FillRectangle($hairLight, $x + 12, $y + 8 + $b, 6, 1)
                Draw-Ears $x $y $b $false
            }
        }
    }
}

# 오른쪽 보기(2번 줄) = 왼쪽 보기(1번 줄)를 좌우로 뒤집은 것
# (1번 줄에서 읽고 2번 줄에 쓰니까 서로 겹치지 않아요. 그리던 붓은 먼저 마무리해요.)
$g.Flush()
for ($col = 0; $col -lt 4; $col++) {
    for ($py = 0; $py -lt $fh; $py++) {
        for ($px = 0; $px -lt $fw; $px++) {
            $bmp.SetPixel($col * $fw + $px, 2 * $fh + $py, $bmp.GetPixel($col * $fw + ($fw - 1 - $px), $fh + $py))
        }
    }
}

# 4번 줄: 캣닢 쿠션 동작 (13b). 쿠션 위에 앉으니까 발 자리(y=46)보다 조금 위에 엉덩이가 와요.
# 앉기: 몸 전체가 8픽셀 내려가고, 치마가 넓게 퍼지고, 부츠 끝이 앞으로 살짝 보여요. 손은 무릎 위.
function Draw-Sitting([int]$x, [int]$y, [string]$eyes) {
    $s = 8                                                         # 앉아서 내려간 만큼
    Draw-FrontBackHair $x $y $s 0
    $g.FillEllipse($dress, $x + 6, $y + 34, 20, 11)                # 넓게 퍼진 치마
    $g.FillRectangle($dressDark, $x + 19, $y + 37, 4, 5)           # 치마 그늘
    $g.FillRectangle($trim, $x + 8, $y + 43, 16, 1)                # 흰 밑단
    $g.FillRectangle($boots, $x + 10, $y + 43, 5, 3)               # 앞으로 나온 부츠 끝
    $g.FillRectangle($boots, $x + 17, $y + 43, 5, 3)
    $g.FillRectangle($dress, $x + 11, $y + 22 + $s, 10, 6)         # 윗몸
    $g.FillRectangle($trim, $x + 12, $y + 22 + $s, 8, 1)           # 흰 깃
    $g.FillRectangle($ribbon, $x + 15, $y + 23 + $s, 2, 2)         # 빨간 리본
    $g.FillRectangle($dress, $x + 9, $y + 22 + $s, 3, 5)           # 소매 (몸 옆)
    $g.FillRectangle($dress, $x + 20, $y + 22 + $s, 3, 5)
    $g.FillRectangle($skin, $x + 11, $y + 36, 3, 2)                # 무릎 위에 모은 손
    $g.FillRectangle($skin, $x + 18, $y + 36, 3, 2)
    Draw-FrontHead $x $y $s $eyes
}
# 웅크리기: 무릎을 끌어안고 머리를 무릎에 기댄 채 졸아요. 머리가 더 내려오고 눈을 감아요.
function Draw-Curled([int]$x, [int]$y) {
    $s = 13                                                        # 머리가 내려온 만큼
    Draw-FrontBackHair $x $y ($s - 2) 0
    $g.FillEllipse($dress, $x + 6, $y + 33, 20, 12)                # 동그랗게 모은 무릎과 치마
    $g.FillRectangle($dressDark, $x + 18, $y + 37, 5, 5)
    $g.FillRectangle($trim, $x + 8, $y + 43, 16, 1)
    $g.FillRectangle($boots, $x + 11, $y + 43, 4, 3)
    $g.FillRectangle($boots, $x + 17, $y + 43, 4, 3)
    $g.FillRectangle($dress, $x + 7, $y + 34, 4, 5)                # 무릎을 감싼 팔 (소매)
    $g.FillRectangle($dress, $x + 21, $y + 34, 4, 5)
    $g.FillRectangle($skin, $x + 11, $y + 37, 10, 2)               # 무릎 앞에서 깍지 낀 손
    Draw-FrontHead $x $y $s 'closed'
}
# 기지개: 까치발로 서서 두 팔을 머리 위로 쭉 뻗고, 눈을 웃듯 감아요.
function Draw-Stretching([int]$x, [int]$y) {
    $b = -1                                                        # 까치발로 1픽셀 올라가요
    Draw-FrontBackHair $x $y $b 0
    Draw-Body $x $y $b 0 0 $true
    $g.FillRectangle($dress, $x + 5, $y + 10 + $b, 3, 13)          # 위로 뻗은 팔 (소매)
    $g.FillRectangle($dress, $x + 24, $y + 10 + $b, 3, 13)
    $g.FillRectangle($skin, $x + 5, $y + 5 + $b, 3, 5)             # 머리 위의 손
    $g.FillRectangle($skin, $x + 24, $y + 5 + $b, 3, 5)
    Draw-FrontHead $x $y $b 'happy'
}
Draw-Sitting (0 * $fw) (4 * $fh) 'open'
Draw-Curled (1 * $fw) (4 * $fh)
Draw-Stretching (2 * $fw) (4 * $fh)
Draw-Sitting (3 * $fw) (4 * $fh) 'happy'

# 바깥 테두리: 투명(분홍) 칸 중에서 바로 옆(위·아래·왼쪽·오른쪽)에 그림이 있는 칸을 진한 색으로 칠해요.
# 테두리가 있으면 풀밭 위에서도 캐릭터가 또렷하게 보여요. (명세 6.1)
# 다른 칸(프레임)의 그림이 넘어와 붙지 않게, 칸 안에서만 이웃을 봐요.
$key = $transparent.ToArgb()
$outline = New-Object System.Collections.Generic.List[object]
for ($py = 0; $py -lt $bmp.Height; $py++) {
    for ($px = 0; $px -lt $bmp.Width; $px++) {
        if ($bmp.GetPixel($px, $py).ToArgb() -ne $key) { continue }
        $fx = $px % $fw; $fy = $py % $fh
        foreach ($d in @(@(-1, 0), @(1, 0), @(0, -1), @(0, 1))) {
            $nx = $fx + $d[0]; $ny = $fy + $d[1]
            if ($nx -lt 0 -or $ny -lt 0 -or $nx -ge $fw -or $ny -ge $fh) { continue }
            if ($bmp.GetPixel($px + $d[0], $py + $d[1]).ToArgb() -ne $key) {
                $outline.Add(@($px, $py)); break
            }
        }
    }
}
foreach ($p in $outline) { $bmp.SetPixel($p[0], $p[1], $ink) }

Save-Bmp $bmp (Join-Path $content 'sprites\test_hero.bmp')
$g.Dispose(); $bmp.Dispose()

# ------------------------------------------------------------ 캣닢 쿠션 ----
# 쿠션 그림 판 (128x32, 칸 32x32, 13b). 0 = 시든 쿠션, 1 = 피어난 쿠션, 2 = "z" (졸음), 3 = 하트.
# 발 위치(주인공이 앉는 자리)는 칸의 (16, 28). 게임 코드(cushion_view)의 칸 크기·발 위치와 같아요.
# 쿠션은 동그랗고 폭신한 방석이고, 위에 캣닢 잎이 꽂혀 있어요. 시들면 색이 바래고 잎이 축 처져요.
$canvas = New-Canvas 128 32
$bmp = $canvas[0]; $g = $canvas[1]

# 쿠션 하나. ox: 칸 왼쪽, bloom: 피었나 (색과 잎 모양이 달라요)
function Draw-Cushion([int]$ox, [bool]$bloom) {
    if ($bloom) {
        $edge = Brush 92 58 108; $side = Brush 170 112 178; $top = Brush 214 160 216; $shine = Brush 244 214 240
        $leaf = Brush 92 170 96; $leafLight = Brush 150 214 128; $flower = Brush 196 160 244
    } else {
        $edge = Brush 70 62 72; $side = Brush 126 112 122; $top = Brush 158 144 152; $shine = Brush 182 170 176
        $leaf = Brush 120 108 70; $leafLight = Brush 148 134 90; $flower = $null
    }
    $g.FillEllipse($edge, $ox + 3, 14, 26, 16)                 # 짙은 테두리 (바닥 그늘 포함)
    $g.FillEllipse($side, $ox + 4, 15, 24, 13)                 # 옆면
    $g.FillEllipse($top, $ox + 5, 14, 22, 10)                  # 폭신한 윗면
    $g.FillRectangle($shine, $ox + 9, 16, 6, 1)                # 윗면 반짝임
    $g.FillRectangle($edge, $ox + 15, 18, 2, 2)                # 가운데 단추 (방석 바느질)
    $g.FillRectangle($edge, $ox + 6, 19, 1, 1); $g.FillRectangle($edge, $ox + 25, 19, 1, 1)  # 양옆 술
    if ($bloom) {  # 피어난 캣닢: 위로 선 잎 세 갈래와 연보라 꽃송이
        Poly $leaf @(($ox + 10), 17, ($ox + 8), 7, ($ox + 12), 15)
        Poly $leaf @(($ox + 16), 16, ($ox + 16), 4, ($ox + 18), 15)
        Poly $leaf @(($ox + 21), 17, ($ox + 24), 8, ($ox + 22), 16)
        $g.FillRectangle($leafLight, $ox + 9, 9, 1, 4); $g.FillRectangle($leafLight, $ox + 16, 6, 1, 6)
        $g.FillRectangle($leafLight, $ox + 23, 10, 1, 4)
        $g.FillRectangle($flower, $ox + 15, 3, 3, 3); $g.FillRectangle($flower, $ox + 7, 6, 2, 2)
        $g.FillRectangle($flower, $ox + 23, 7, 2, 2)
        $g.FillRectangle((Brush 240 226 255), $ox + 16, 3, 1, 1)  # 꽃 가운데 반짝
    } else {       # 시든 캣닢: 옆으로 축 처진 잎
        Poly $leaf @(($ox + 12), 16, ($ox + 6), 13, ($ox + 11), 14)
        Poly $leaf @(($ox + 16), 15, ($ox + 15), 9, ($ox + 18), 14)
        Poly $leaf @(($ox + 20), 16, ($ox + 26), 14, ($ox + 21), 14)
        $g.FillRectangle($leafLight, $ox + 15, 10, 1, 3)
    }
}
Draw-Cushion 0 $false
Draw-Cushion 32 $true
# 2번 칸: 작은 "z" (6x6, 크림색 + 진한 테두리). 쉬는 동안 머리 위로 둥실 떠올라요.
$zInk = Brush 60 52 84; $zFill = Brush 244 238 214
# 먼저 진한 색으로 한 픽셀 두껍게 (테두리), 그 위에 크림색 한 픽셀 선
$g.FillRectangle($zInk, 75, 12, 8, 3); $g.FillRectangle($zInk, 75, 17, 8, 3)  # 위·아래 가로줄
$g.FillRectangle($zInk, 79, 13, 3, 3); $g.FillRectangle($zInk, 78, 14, 3, 3)  # 사선
$g.FillRectangle($zInk, 77, 15, 3, 3); $g.FillRectangle($zInk, 76, 16, 3, 3)
$g.FillRectangle($zFill, 76, 13, 6, 1); $g.FillRectangle($zFill, 80, 14, 1, 1); $g.FillRectangle($zFill, 79, 15, 1, 1)
$g.FillRectangle($zFill, 78, 16, 1, 1); $g.FillRectangle($zFill, 77, 17, 1, 1); $g.FillRectangle($zFill, 76, 18, 6, 1)
# 3번 칸: 하트 (7x6, 분홍 + 진한 테두리). 앉는 순간 퐁 하고 떠올라요.
$hInk = Brush 96 36 64; $hFill = Brush 244 120 160; $hShine = Brush 255 214 226
Poly $hInk @(108, 12, 112, 11, 116, 12, 116, 15, 112, 20, 108, 15)
Poly $hFill @(109, 13, 111, 12, 112, 13, 113, 12, 115, 13, 115, 15, 112, 18, 109, 15)
$g.FillRectangle($hShine, 110, 13, 1, 1)
Save-Bmp $bmp (Join-Path $content 'props\catnip_cushion.bmp')
$g.Dispose(); $bmp.Dispose()

# ------------------------------------------------------------ 숲 박쥐 (13c) ----
# 박쥐 그림 판 (48x24, 칸 24x24): 0 = 날개를 편 보통, 1 = 공격 예고(날개를 접고 몸을 웅크림).
# 발 위치는 칸의 (12, 22): 박쥐는 떠 있어서 몸은 발보다 한참 위(y 8~16)에 그려요. monsters/bat.json의 "origin"과 같아요.
$canvas = New-Canvas 48 24
$bmp = $canvas[0]; $g = $canvas[1]
$batInk = Brush 34 24 52; $batBody = Brush 86 66 120; $batWing = Brush 64 48 96; $batBelly = Brush 128 104 156
$batEye = Brush 255 210 90
function Draw-Bat([int]$ox, [bool]$folded) {
    if ($folded) {  # 예고: 날개를 몸에 붙이고 작게 웅크려요
        Poly $batInk @(($ox + 6), 10, ($ox + 12), 6, ($ox + 18), 10, ($ox + 16), 18, ($ox + 8), 18)
        Poly $batWing @(($ox + 7), 10, ($ox + 12), 7, ($ox + 17), 10, ($ox + 15), 17, ($ox + 9), 17)
    } else {        # 보통: 양옆으로 뾰족뾰족한 날개를 활짝
        Poly $batInk @(($ox + 1), 8, ($ox + 9), 9, ($ox + 9), 15, ($ox + 5), 13, ($ox + 3), 15)
        Poly $batInk @(($ox + 23), 8, ($ox + 15), 9, ($ox + 15), 15, ($ox + 19), 13, ($ox + 21), 15)
        Poly $batWing @(($ox + 2), 9, ($ox + 9), 10, ($ox + 9), 14, ($ox + 5), 12, ($ox + 3), 13)
        Poly $batWing @(($ox + 22), 9, ($ox + 15), 10, ($ox + 15), 14, ($ox + 19), 12, ($ox + 21), 13)
    }
    $g.FillEllipse($batInk, $ox + 8, 7, 9, 10)                 # 몸 테두리
    $g.FillEllipse($batBody, $ox + 9, 8, 7, 8)                 # 몸
    $g.FillRectangle($batBelly, $ox + 11, 12, 3, 3)            # 배
    Poly $batInk @(($ox + 9), 8, ($ox + 9), 4, ($ox + 11), 7)  # 뾰족 귀
    Poly $batInk @(($ox + 15), 8, ($ox + 16), 4, ($ox + 14), 7)
    $g.FillRectangle($batEye, $ox + 10, 10, 1, 1)              # 노란 눈
    $g.FillRectangle($batEye, $ox + 14, 10, 1, 1)
    $g.FillRectangle((Brush 240 240 240), $ox + 12, 13, 1, 1)  # 송곳니
}
Draw-Bat 0 $false
Draw-Bat 24 $true
Save-Bmp $bmp (Join-Path $content 'monsters\bat.bmp')
$g.Dispose(); $bmp.Dispose()

# ---------------------------------------------------------- 가시 버섯 (13c) ----
# 버섯 그림 판 (64x32, 칸 32x32): 0 = 보통(동그란 갓), 1 = 공격 예고(갓이 부풀고 가시가 삐죽).
# 발 위치는 칸의 (16, 29). monsters/mushroom.json의 "origin"과 같아요.
$canvas = New-Canvas 64 32
$bmp = $canvas[0]; $g = $canvas[1]
$shInk = Brush 52 30 30; $shCap = Brush 196 74 64; $shCapLight = Brush 232 120 96; $shDot = Brush 248 236 210
$shStem = Brush 236 222 190; $shStemShade = Brush 196 180 150; $shSpike = Brush 250 246 230
function Draw-Mushroom([int]$ox, [bool]$angry) {
    $puff = 0                                                   # 갓 크기는 그대로 (예고 때는 둘레에 가시가 나와요)
    $g.FillRectangle($shInk, $ox + 11, 18, 10, 12)              # 기둥 테두리
    $g.FillRectangle($shStem, $ox + 12, 18, 8, 11)              # 기둥
    $g.FillRectangle($shStemShade, $ox + 17, 19, 3, 10)         # 기둥 그늘
    $g.FillRectangle($shInk, $ox + 13, 23, 2, 3)                # 눈 (기둥에 있어요)
    $g.FillRectangle($shInk, $ox + 17, 23, 2, 3)
    if ($angry) {
        $g.FillRectangle($shInk, $ox + 12, 22, 3, 1)            # 찡그린 눈썹
        $g.FillRectangle($shInk, $ox + 17, 22, 3, 1)
    }
    $g.FillEllipse($shInk, $ox + 3 - $puff, 4 - $puff, 26 + $puff * 2, 17 + $puff)   # 갓 테두리
    $g.FillEllipse($shCap, $ox + 4 - $puff, 5 - $puff, 24 + $puff * 2, 15 + $puff)   # 빨간 갓
    $g.FillEllipse($shCapLight, $ox + 8, 6 - $puff, 10, 5)                           # 갓 반짝임
    $g.FillEllipse($shDot, $ox + 7, 11, 4, 3); $g.FillEllipse($shDot, $ox + 15, 8, 5, 3)   # 흰 점
    $g.FillEllipse($shDot, $ox + 22, 12, 4, 3)
    $g.FillRectangle($shInk, $ox + 6, 18, 20, 1)                # 갓 밑 그늘선
    if ($angry) {  # 갓 가장자리에서 바깥으로 삐죽 나온 흰 가시 다섯 개 (갓 위에 그려야 보여요)
        # 갓(x 3~29, y 4~21) 가장자리 안쪽에 밑변, 칸 끝(0·31, 위 0)에 뾰족한 끝
        foreach ($s in @(@(5, 10, 5, 15, 0, 12), @(8, 6, 11, 4, 4, 1), @(14, 5, 18, 5, 16, 0),
                         @(21, 4, 24, 6, 28, 1), @(27, 10, 27, 15, 31, 12))) {
            Poly $shInk @(($ox + $s[0] - 1), $s[1], ($ox + $s[2] + 1), $s[3], ($ox + $s[4]), $s[5])  # 테두리
            Poly $shSpike @(($ox + $s[0]), $s[1], ($ox + $s[2]), $s[3], ($ox + $s[4]), $s[5])      # 흰 가시
        }
    }
}
Draw-Mushroom 0 $false
Draw-Mushroom 32 $true
Save-Bmp $bmp (Join-Path $content 'monsters\mushroom.bmp')
$g.Dispose(); $bmp.Dispose()

# ----------------------------------------------------- 침 뱉는 개구리 (13c) ----
# 개구리 그림 판 (48x24, 칸 24x24): 0 = 보통(앉음), 1 = 공격 예고(볼을 부풀리고 입을 벌림).
# 발 위치는 칸의 (12, 21). monsters/frog.json의 "origin"과 같아요.
$canvas = New-Canvas 48 24
$bmp = $canvas[0]; $g = $canvas[1]
$frInk = Brush 24 52 36; $frBody = Brush 90 168 96; $frBelly = Brush 214 226 150; $frEye = Brush 250 250 240
$frPupil = Brush 20 24 30; $frCheek = Brush 230 140 120
function Draw-Frog([int]$ox, [bool]$puffed) {
    $g.FillEllipse($frInk, $ox + 3, 9, 18, 13)                  # 몸 테두리
    $g.FillEllipse($frBody, $ox + 4, 10, 16, 11)                # 몸
    $g.FillEllipse($frBelly, $ox + 8, 14, 8, 6)                 # 배
    $g.FillRectangle($frInk, $ox + 3, 19, 5, 2); $g.FillRectangle($frInk, $ox + 16, 19, 5, 2)   # 발
    foreach ($ex in @(6, 14)) {                                 # 툭 튀어나온 두 눈
        $g.FillEllipse($frInk, $ox + $ex - 1, 5, 6, 6)
        $g.FillEllipse($frEye, $ox + $ex, 6, 4, 4)
        $g.FillRectangle($frPupil, $ox + $ex + 1, 7, 2, 2)
    }
    if ($puffed) {  # 예고: 볼이 부풀고 입을 벌려요 (곧 침이 날아와요)
        $g.FillEllipse($frCheek, $ox + 3, 13, 4, 4); $g.FillEllipse($frCheek, $ox + 17, 13, 4, 4)
        $g.FillEllipse($frInk, $ox + 9, 12, 6, 4)
        $g.FillEllipse((Brush 200 70 80), $ox + 10, 13, 4, 2)
    } else {
        $g.FillRectangle($frInk, $ox + 9, 13, 6, 1)             # 다문 입
    }
}
Draw-Frog 0 $false
Draw-Frog 24 $true
Save-Bmp $bmp (Join-Path $content 'monsters\frog.bmp')
$g.Dispose(); $bmp.Dispose()

# ------------------------------------------------------- 아이템 아이콘 (14b) ----
# 아이템 아이콘 그림 판 (160x16, 칸 16x16). 칸 번호는 items/catalog.json의 "icon"과 같아요.
#   0 회복 물약(빨강)  1 별빛 물약(파랑)  2 슬라임 젤리  3 박쥐 날개  4 버섯 가시  5 개구리 침샘  6 롱소드  7 단검  8 대검  9 (빈칸)
# 바닥에 떨어진 아이템·알림 줄·인벤토리 화면(14c)이 함께 써요. 테두리는 진한 잉크색으로 둘러서 풀밭 위에서도 잘 보여요.
$canvas = New-Canvas 160 16
$bmp = $canvas[0]; $g = $canvas[1]
$icInk = Brush 28 22 40; $icGlass = Brush 220 232 240; $icCork = Brush 150 100 64; $icShine = Brush 255 255 255
function Draw-Potion([int]$ox, $liquid, $liquidDark) {
    $g.FillRectangle($icInk, $ox + 5, 1, 6, 5)                  # 목 테두리
    $g.FillRectangle($icCork, $ox + 6, 1, 4, 2)                 # 코르크 마개
    $g.FillRectangle($icGlass, $ox + 6, 3, 4, 3)                # 유리 목
    $g.FillEllipse($icInk, $ox + 2, 5, 12, 11)                  # 둥근 병 테두리
    $g.FillEllipse($icGlass, $ox + 3, 6, 10, 9)                 # 유리
    $g.FillEllipse($liquidDark, $ox + 3, 8, 10, 7)              # 물약 (아래쪽 진한 색)
    $g.FillEllipse($liquid, $ox + 4, 8, 8, 5)                   # 물약 (밝은 색)
    $g.FillRectangle($icShine, $ox + 5, 7, 2, 2)                # 유리 반짝임
}
Draw-Potion 0 (Brush 226 64 72) (Brush 150 30 48)               # 회복 물약: 붉은 약초
Draw-Potion 16 (Brush 92 150 240) (Brush 44 76 170)             # 별빛 물약: 밤이슬
$g.FillRectangle((Brush 255 236 140), 16 + 9, 10, 1, 1)         # 별빛 물약 안의 작은 별
# 2 슬라임 젤리: 말랑한 민트색 덩어리
$g.FillEllipse($icInk, 32 + 2, 4, 12, 11)
$g.FillEllipse((Brush 96 200 150), 32 + 3, 5, 10, 9)
$g.FillEllipse((Brush 150 236 190), 32 + 4, 6, 6, 4)
$g.FillRectangle($icShine, 32 + 5, 7, 2, 1)
# 3 박쥐 날개: 뾰족뾰족한 보라색 날개 한 장
Poly $icInk @((48 + 1), 3, (48 + 14), 5, (48 + 13), 12, (48 + 10), 10, (48 + 7), 13, (48 + 5), 10, (48 + 2), 12)
Poly (Brush 92 70 136) @((48 + 2), 4, (48 + 13), 6, (48 + 12), 11, (48 + 10), 9, (48 + 7), 12, (48 + 5), 9, (48 + 3), 10)
$g.FillRectangle((Brush 132 108 170), 48 + 3, 5, 9, 1)          # 날개 뼈
# 4 버섯 가시: 붉은 밑동에서 솟은 흰 가시
Poly $icInk @((64 + 8), 0, (64 + 13), 13, (64 + 3), 13)
Poly (Brush 250 246 230) @((64 + 8), 2, (64 + 12), 12, (64 + 4), 12)
$g.FillRectangle($icInk, 64 + 2, 12, 12, 4)
$g.FillRectangle((Brush 196 74 64), 64 + 3, 13, 10, 2)          # 갓 조각
$g.FillRectangle((Brush 248 236 210), 64 + 5, 13, 2, 1)
# 5 개구리 침샘: 초록 주머니에 분홍 반점
$g.FillEllipse($icInk, 80 + 2, 3, 12, 12)
$g.FillEllipse((Brush 110 184 104), 80 + 3, 4, 10, 10)
$g.FillEllipse((Brush 230 140 150), 80 + 5, 7, 3, 3)
$g.FillEllipse((Brush 230 140 150), 80 + 9, 9, 2, 2)
$g.FillRectangle($icShine, 80 + 5, 5, 2, 1)
# 6 롱소드: 왼쪽 아래 손잡이에서 오른쪽 위로 뻗은 은빛 칼날
Poly $icInk @((96 + 4), 10, (96 + 13), 1, (96 + 15), 1, (96 + 15), 3, (96 + 6), 12)
Poly (Brush 214 222 236) @((96 + 5), 10, (96 + 13), 2, (96 + 14), 2, (96 + 14), 3, (96 + 6), 11)
$g.FillRectangle((Brush 255 255 255), 96 + 11, 4, 1, 1)         # 칼날 반짝임
Poly $icInk @((96 + 2), 8, (96 + 4), 8, (96 + 8), 12, (96 + 8), 14, (96 + 6), 14, (96 + 2), 10)
Poly (Brush 226 182 80) @((96 + 3), 9, (96 + 4), 9, (96 + 7), 12, (96 + 7), 13, (96 + 6), 13, (96 + 3), 10)   # 금빛 가드
$g.FillRectangle($icInk, 96 + 1, 12, 4, 4)
$g.FillRectangle((Brush 120 76 48), 96 + 2, 13, 2, 2)           # 손잡이
# 7 단검 (15a): 짧고 가는 칼날, 손잡이가 길어 보여요
Poly $icInk @((112 + 6), 10, (112 + 12), 4, (112 + 14), 4, (112 + 14), 6, (112 + 8), 12)
Poly (Brush 214 222 236) @((112 + 7), 10, (112 + 12), 5, (112 + 13), 5, (112 + 13), 6, (112 + 8), 11)
$g.FillRectangle((Brush 255 255 255), 112 + 11, 6, 1, 1)        # 칼날 반짝임
$g.FillRectangle($icInk, 112 + 4, 9, 5, 2); $g.FillRectangle($icInk, 112 + 5, 8, 2, 5)
$g.FillRectangle((Brush 170 176 190), 112 + 5, 9, 3, 1)         # 은빛 가드
$g.FillRectangle($icInk, 112 + 1, 11, 5, 5)
$g.FillRectangle((Brush 70 54 90), 112 + 2, 12, 3, 3)           # 보라 가죽 손잡이
# 8 대검 (15a): 칸을 거의 꽉 채우는 넓고 무거운 칼날
Poly $icInk @((128 + 3), 10, (128 + 12), 0, (128 + 16), 0, (128 + 16), 4, (128 + 6), 13)
Poly (Brush 188 198 214) @((128 + 4), 10, (128 + 12), 1, (128 + 15), 1, (128 + 15), 4, (128 + 6), 12)
Poly (Brush 236 242 250) @((128 + 5), 10, (128 + 13), 1, (128 + 14), 1, (128 + 6), 10)   # 날 밝은 면
$g.FillRectangle($icInk, 128 + 1, 8, 7, 3); $g.FillRectangle($icInk, 128 + 3, 7, 3, 7)
$g.FillRectangle((Brush 140 70 60), 128 + 2, 9, 5, 1)           # 붉은 쇠 가드
$g.FillRectangle($icInk, 128 + 0, 12, 4, 4)
$g.FillRectangle((Brush 96 64 44), 128 + 1, 13, 2, 2)           # 손잡이
Save-Bmp $bmp (Join-Path $content 'items\item_icons.bmp')
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
# 흙길 왼쪽 끝 벽에 동굴 입구를 뚫어요. 입구 칸(0, 12)을 밟으면 시험 동굴로 가요.
$ground[(Idx 1 12)] = 3
$ground[(Idx 0 12)] = 12
# 흙길 오른쪽 끝 벽을 터서 숲으로 가는 길을 내요. 끝 칸(39, 12)을 밟으면 시험 숲으로 가요.
$ground[(Idx 38 12)] = 3
$ground[(Idx 39 12)] = 3

# 나무: (x, y)에 밑동을 놓고, 바로 위 칸 (x, y-1)에 나뭇잎을 놓아요.
$trees = @(@(5,5), @(9,4), @(14,7), @(24,5), @(30,8), @(35,5), @(6,20), @(11,24),
           @(16,18), @(23,22), @(36,15), @(34,27), @(4,27), @(8,15), @(13,4))
foreach ($t in $trees) {
    $objects[(Idx $t[0] $t[1])] = 6
    $canopy[(Idx $t[0] ($t[1] - 1))] = 7
}
# 바위
foreach ($r in @(@(12,15), @(28,14), @(17,26), @(8,9), @(31,3))) { $objects[(Idx $r[0] $r[1])] = 8 }
# 가로등: 흙길 바로 위 칸에 네 개. 밤에만 켜지는 빛(night_only)을 같은 자리에 둬요.
$lanterns = @(@(10,11), @(17,11), @(26,11), @(34,11))
foreach ($l in $lanterns) { $objects[(Idx $l[0] $l[1])] = 13 }
$lanternLights = foreach ($l in $lanterns) {
    '      { "x": ' + $l[0] + '.5, "y": ' + $l[1] + '.2, "radius": 100, "color": "#ffd890", "intensity": 0.95, "flicker": 0.08, "night_only": true }'
}

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
  "display_name": "시험 초원",
  "tileset": "tilesets/test_tiles.json",
  "tile_size": 32,
  "width": $W,
  "height": $H,
  "spawn_points": [
    { "id": "default", "x": 6, "y": 12 },
    { "id": "from_cave", "x": 2, "y": 12 },
    { "id": "from_forest", "x": 37, "y": 12 }
  ],
  "exits": [
    { "area": { "x": 0, "y": 12, "w": 1, "h": 1 }, "to_map": "maps/test_cave.json", "to_spawn": "entrance", "transition": "iris" },
    { "area": { "x": 39, "y": 12, "w": 1, "h": 1 }, "to_map": "maps/test_forest.json", "to_spawn": "from_meadow", "transition": "fade" }
  ],
  "entities": [
    { "type": "training_dummy", "id": "meadow_dummy_1", "x": 10, "y": 14, "data": "props/training_dummy.json" },
    { "type": "training_dummy", "id": "meadow_dummy_2", "x": 13, "y": 14, "data": "props/training_dummy.json" },
    { "type": "monster", "id": "meadow_slime_1", "x": 11, "y": 19, "data": "monsters/slime_green.json" },
    { "type": "monster", "id": "meadow_slime_2", "x": 15, "y": 21, "data": "monsters/slime_green.json" }
  ],
  "ambience": {
    "ambient_fx": ["fx/petals.json"],
    "night_fx": ["fx/fireflies.json"],
    "day_cycle": "time/day_cycle.json",
    "weather": "weather/meadow.json",
    "lights": [
$($lanternLights -join ",`n")
    ]
  },
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

# --------------------------------------------------------------- 시험 동굴 ----
# 조명을 시험하는 어두운 동굴 맵 (가로 30칸 x 세로 20칸). 바닥은 흙, 벽은 돌담.
# 횃불 타일을 놓고, 같은 자리에 "lights"로 빛을 둬요. 연못 옆에는 푸른 마법 빛이 숨 쉬듯 깜빡여요.
$W = 30; $H = 20
$ground = New-Object 'int[]' ($W * $H)
$objects = New-Object 'int[]' ($W * $H)
$rng = New-Object System.Random 99

for ($y = 0; $y -lt $H; $y++) {
    for ($x = 0; $x -lt $W; $x++) {
        $ground[(Idx $x $y)] = if ($rng.NextDouble() -lt 0.12) { 10 } else { 3 }
    }
}
# 가장자리 벽과 안쪽 벽 덩어리 (굴 모양을 만들어요)
for ($x = 0; $x -lt $W; $x++) { $ground[(Idx $x 0)] = 5; $ground[(Idx $x ($H - 1))] = 5 }
for ($y = 0; $y -lt $H; $y++) { $ground[(Idx 0 $y)] = 5; $ground[(Idx ($W - 1) $y)] = 5 }
foreach ($block in @(@(8, 1, 3, 6), @(8, 12, 3, 7), @(17, 5, 2, 9), @(23, 1, 6, 4))) {
    for ($y = $block[1]; $y -lt $block[1] + $block[3]; $y++) {
        for ($x = $block[0]; $x -lt $block[0] + $block[2]; $x++) { $ground[(Idx $x $y)] = 5 }
    }
}
# 왼쪽 벽의 출구 (0, 10): 밟으면 초원의 동굴 입구 앞으로 돌아가요.
$ground[(Idx 0 10)] = 12
$ground[(Idx 1 10)] = 3
# 지하 연못
for ($y = 13; $y -le 16; $y++) { for ($x = 22; $x -le 26; $x++) { $ground[(Idx $x $y)] = 4 } }
# 바위
foreach ($r in @(@(4,4), @(13,15), @(20,2), @(26,9), @(5,16))) { $objects[(Idx $r[0] $r[1])] = 8 }
# 횃불 자리 (빛도 같은 자리에 둬요)
$torches = @(@(3,2), @(6,9), @(12,3), @(14,11), @(21,7), @(27,11), @(19,17), @(3,17))
foreach ($t in $torches) { $objects[(Idx $t[0] $t[1])] = 11 }

# 빛 목록: 횃불 불꽃은 칸의 위쪽(y + 0.2)에 있어요.
$lightLines = foreach ($t in $torches) {
    '      { "x": ' + $t[0] + '.5, "y": ' + $t[1] + '.2, "radius": 110, "color": "#ffb060", "intensity": 1, "flicker": 0.35, "fx": "fx/torch_embers.json" }'
}
$lightLines += '      { "x": 24.5, "y": 14.5, "radius": 140, "color": "#5878ff", "intensity": 0.9, "pulse": { "frequency": 0.4, "depth": 0.5 } }'

$layers = @(
    (Format-Layer 'ground' 'ground' $ground),
    (Format-Layer 'objects' 'low_objects' $objects)
) -join ",`n"

$json = @"
{
  "schema_version": 1,
  "map_id": "test_cave",
  "display_name": "시험 동굴",
  "tileset": "tilesets/test_tiles.json",
  "tile_size": 32,
  "width": $W,
  "height": $H,
  "spawn_points": [
    { "id": "default", "x": 4, "y": 10 },
    { "id": "entrance", "x": 2, "y": 10 }
  ],
  "exits": [
    { "area": { "x": 0, "y": 10, "w": 1, "h": 1 }, "to_map": "maps/test_meadow.json", "to_spawn": "from_cave", "transition": "iris" }
  ],
  "ambience": {
    "ambient_light": "#1c1a30",
    "player_light": { "radius": 80, "color": "#ffe2b8", "intensity": 0.55 },
    "lights": [
$($lightLines -join ",`n")
    ]
  },
  "layers": [
$layers
  ]
}
"@

$mapPath = Join-Path $content 'maps\test_cave.json'
[System.IO.File]::WriteAllText($mapPath, ($json -replace "`r`n", "`n") + "`n", (New-Object System.Text.UTF8Encoding($false)))
Write-Host "wrote $mapPath"

# ---------------------------------------------------------------- 시험 숲 ----
# 바람 흔들림을 시험하는 숲 맵 (가로 36칸 x 세로 24칸). 초원 오른쪽 끝 길로 들어와요.
# 숲 바닥에 키 큰 풀(흔들림)과 버섯, 짙은 나뭇잎 나무(흔들림)가 빽빽하고,
# 가운데로 시냇물(반짝임)이 흘러요. 흙길은 시냇물 위 징검다리로 건너요.
# 가장자리는 덤불로 막혀 있고, 왼쪽 끝 흙길 칸(0, 12)을 밟으면 초원으로 돌아가요.
$W = 36; $H = 24
$ground = New-Object 'int[]' ($W * $H)
$decor = New-Object 'int[]' ($W * $H)
$objects = New-Object 'int[]' ($W * $H)
$canopy = New-Object 'int[]' ($W * $H)
$rng = New-Object System.Random 314

for ($y = 0; $y -lt $H; $y++) {
    for ($x = 0; $x -lt $W; $x++) {
        $ground[(Idx $x $y)] = if ($rng.NextDouble() -lt 0.2) { 2 } else { 19 }
    }
}
# 흙길: 왼쪽 입구에서 오른쪽으로, 가운데쯤에서 아래로 굽어요.
for ($x = 0; $x -le 26; $x++) { $ground[(Idx $x 12)] = 3 }
for ($y = 12; $y -le 19; $y++) { $ground[(Idx 26 $y)] = 3 }
for ($x = 26; $x -le 32; $x++) { $ground[(Idx $x 19)] = 3 }
# 시냇물: x = 17~18, 위에서 아래로. 흙길이 지나는 줄(12)은 징검다리(흙)로 남겨요.
for ($y = 1; $y -lt $H - 1; $y++) {
    if ($y -ne 12) { $ground[(Idx 17 $y)] = 4; $ground[(Idx 18 $y)] = 4 }
}
# 가장자리 덤불 (입구 칸은 비워요)
for ($x = 0; $x -lt $W; $x++) { $objects[(Idx $x 0)] = 18; $objects[(Idx $x ($H - 1))] = 18 }
for ($y = 0; $y -lt $H; $y++) { $objects[(Idx 0 $y)] = 18; $objects[(Idx ($W - 1) $y)] = 18 }
$objects[(Idx 0 12)] = 0

# 이 칸에 나무를 심어도 되나요? 길·물·가장자리·다른 나무와 붙지 않아야 해요.
function Can-Plant([int]$x, [int]$y) {
    if ($x -lt 2 -or $x -gt $W - 3 -or $y -lt 2 -or $y -gt $H - 2) { return $false }
    for ($dy = -1; $dy -le 1; $dy++) {
        for ($dx = -1; $dx -le 1; $dx++) {
            $i = Idx ($x + $dx) ($y + $dy)
            if ($ground[$i] -eq 3 -or $ground[$i] -eq 4 -or $objects[$i] -ne 0) { return $false }
        }
    }
    return $true
}
# 나무: 밑동(6) 위 칸에 짙은 나뭇잎(21). 시작 자리 둘레(2~5, 10~14)는 비워 둬요.
for ($n = 0; $n -lt 400; $n++) {
    $x = $rng.Next(2, $W - 2); $y = $rng.Next(2, $H - 1)
    if ($x -le 5 -and $y -ge 10 -and $y -le 14) { continue }
    if (Can-Plant $x $y) {
        $objects[(Idx $x $y)] = 6
        $canopy[(Idx $x ($y - 1))] = 21
    }
}
# 바닥 장식: 숲 바닥 위 빈칸에 키 큰 풀 16%, 버섯 2%, 꽃 3%
for ($y = 1; $y -lt $H - 1; $y++) {
    for ($x = 1; $x -lt $W - 1; $x++) {
        $i = Idx $x $y
        if (($ground[$i] -eq 19 -or $ground[$i] -eq 2) -and $objects[$i] -eq 0) {
            $roll = $rng.NextDouble()
            if ($roll -lt 0.16) { $decor[$i] = 17 }
            elseif ($roll -lt 0.18) { $decor[$i] = 20 }
            elseif ($roll -lt 0.21) { $decor[$i] = 9 }
        }
    }
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
  "map_id": "test_forest",
  "display_name": "시험 숲",
  "tileset": "tilesets/test_tiles.json",
  "tile_size": 32,
  "width": $W,
  "height": $H,
  "spawn_points": [
    { "id": "default", "x": 3, "y": 12 },
    { "id": "from_meadow", "x": 2, "y": 12 }
  ],
  "exits": [
    { "area": { "x": 0, "y": 12, "w": 1, "h": 1 }, "to_map": "maps/test_meadow.json", "to_spawn": "from_forest", "transition": "fade" }
  ],
  "ambience": {
    "ambient_fx": ["fx/leaves.json"],
    "night_fx": ["fx/fireflies.json"],
    "day_cycle": "time/day_cycle.json",
    "weather": "weather/forest.json"
  },
  "layers": [
$layers
  ]
}
"@

$mapPath = Join-Path $content 'maps\test_forest.json'
[System.IO.File]::WriteAllText($mapPath, ($json -replace "`r`n", "`n") + "`n", (New-Object System.Text.UTF8Encoding($false)))
Write-Host "wrote $mapPath"
