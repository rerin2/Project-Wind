# 이 파일은 무엇을 하나요?
# 시험용 게임 자료를 "그림판처럼" 자동으로 그려 주는 스크립트(명령을 모아 둔 파일)예요.
# 네모, 동그라미 같은 단순한 도형으로 풀밭·나무·캐릭터를 직접 그려요.
# 다른 게임의 그림을 가져다 쓰지 않아서 저작권 걱정이 없어요.
#
# 사용법 (저장소 맨 위 폴더에서):  powershell -NoProfile -File tools\test_content\generate_test_content.ps1
# 만들어지는 파일:
#   content/tilesets/test_tiles.bmp   타일 스티커 판: 32x32 칸, 가로 8개 x 세로 2줄 (1번 = 왼쪽 위)
#   content/sprites/test_hero.bmp     주인공(고양이 귀 은발 소녀) 그림 판: 32x48 칸, 가로 4개(걸음) x 세로 4줄(아래·왼쪽·오른쪽·위)
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
                # 1) 뒷머리: 등 뒤로 허리까지 내려오는 긴 머리 (몸보다 먼저 = 뒤에 그려요)
                $g.FillRectangle($hairShade, $x + 7, $y + 12 + $b, 18, 20)
                $g.FillRectangle($hairShade, $x + 8 + $sway, $y + 32 + $b, 4, 2)
                $g.FillRectangle($hairShade, $x + 20 + $sway, $y + 32 + $b, 4, 2)
                Draw-Body $x $y $b $leftLeg $rightLeg $true
                # 2) 팔: 소매(남색) + 손(살색), 걸을 때 앞뒤로 흔들려요
                $g.FillRectangle($dress, $x + 8, $y + 22 + $b, 3, 4)
                $g.FillRectangle($dress, $x + 21, $y + 22 + $b, 3, 4)
                $g.FillRectangle($skin, $x + 8, $y + 26 + $b - $leftLeg, 3, 6)
                $g.FillRectangle($skin, $x + 21, $y + 26 + $b - $rightLeg, 3, 6)
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
                # 7) 눈: 속눈썹(진한 줄) + 보라색 눈동자 + 반짝이는 점, 볼 터치, 입
                $g.FillRectangle($lash, $x + 11, $y + 14 + $b, 3, 1)
                $g.FillRectangle($lash, $x + 18, $y + 14 + $b, 3, 1)
                $g.FillRectangle($iris, $x + 12, $y + 15 + $b, 2, 3)
                $g.FillRectangle($iris, $x + 18, $y + 15 + $b, 2, 3)
                $g.FillRectangle($shine, $x + 12, $y + 15 + $b, 1, 1)
                $g.FillRectangle($shine, $x + 18, $y + 15 + $b, 1, 1)
                $g.FillRectangle($blush, $x + 10, $y + 19 + $b, 2, 1)
                $g.FillRectangle($blush, $x + 20, $y + 19 + $b, 2, 1)
                $g.FillRectangle($skinShade, $x + 15, $y + 20 + $b, 2, 1)
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
  "ambience": {
    "ambient_fx": ["fx/petals.json", "fx/fireflies.json"]
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
