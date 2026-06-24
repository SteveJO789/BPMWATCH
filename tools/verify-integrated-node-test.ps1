$ErrorActionPreference = 'Stop'

$projectRoot = Join-Path $PSScriptRoot '..\firmware\tests\integrated_node_test'
$errors = [System.Collections.Generic.List[string]]::new()

function Add-MissingContract {
    param([string]$Message)
    $errors.Add("MISSING: $Message")
}

function Require-Match {
    param(
        [string]$Text,
        [string]$Pattern,
        [string]$Description
    )

    if ($Text -notmatch $Pattern) {
        Add-MissingContract $Description
    }
}

function Get-EnvironmentSection {
    param(
        [string]$Text,
        [string]$EnvironmentName
    )

    $escapedName = [regex]::Escape($EnvironmentName)
    $match = [regex]::Match(
        $Text,
        "(?ms)^\s*\[env:$escapedName\]\s*\r?\n(?<body>.*?)(?=^\s*\[|\z)"
    )
    if ($match.Success) {
        return $match.Groups['body'].Value
    }
    return ''
}

if (-not (Test-Path -LiteralPath $projectRoot -PathType Container)) {
    Add-MissingContract 'firmware/tests/integrated_node_test directory'
}

$platformioPath = Join-Path $projectRoot 'platformio.ini'
$sourceRoot = Join-Path $projectRoot 'src'
$requiredFiles = @(
    'platformio.ini'
    'src/NodeConfig.h'
    'src/DiagnosticsState.h'
    'src/DiagnosticsSync.h'
    'src/DiagnosticsSync.cpp'
    'src/EspNowRangeLink.h'
    'src/EspNowRangeLink.cpp'
    'src/Gy511Addresses.h'
    'src/Gy511Status.h'
    'src/I2cScan.h'
    'src/MedianRangeFilter.h'
    'src/Gy511Sensor.h'
    'src/Gy511Sensor.cpp'
    'src/Max30102Sensor.h'
    'src/Max30102Sensor.cpp'
    'src/RadarMap.h'
    'src/UwbDiagnostics.h'
    'src/UwbDiagnostics.cpp'
    'src/UwbEventQueue.h'
    'src/UwbEventQueue.cpp'
    'src/DiagnosticsDisplay.h'
    'src/DiagnosticsDisplay.cpp'
    'src/main.cpp'
)

foreach ($relativePath in $requiredFiles) {
    $path = Join-Path $projectRoot ($relativePath -replace '/', '\')
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        Add-MissingContract $relativePath
    }
}

$platformio = if (Test-Path -LiteralPath $platformioPath -PathType Leaf) {
    Get-Content -Raw -LiteralPath $platformioPath
}
else { '' }

Require-Match $platformio '(?m)^\s*\[env:node_a_anchor\]\s*$' 'PlatformIO environment node_a_anchor'
Require-Match $platformio '(?m)^\s*\[env:node_a_anchor_trace\]\s*$' 'PlatformIO environment node_a_anchor_trace'
Require-Match $platformio '(?m)^\s*\[env:node_a_anchor_espnow_range\]\s*$' 'PlatformIO environment node_a_anchor_espnow_range'
Require-Match $platformio '(?m)^\s*\[env:node_a_anchor_espnow_range_trace\]\s*$' 'PlatformIO environment node_a_anchor_espnow_range_trace'
Require-Match $platformio '(?m)^\s*\[env:node_b_tag\]\s*$' 'PlatformIO environment node_b_tag'
Require-Match $platformio '(?m)^\s*\[env:node_b_tag_trace\]\s*$' 'PlatformIO environment node_b_tag_trace'
Require-Match $platformio '(?m)^\s*\[env:node_b_tag_espnow_display\]\s*$' 'PlatformIO environment node_b_tag_espnow_display'
Require-Match $platformio '(?m)^\s*\[env:node_b_tag_espnow_display_trace\]\s*$' 'PlatformIO environment node_b_tag_espnow_display_trace'
Require-Match $platformio '(?m)^\s*\[env:native\]\s*$' 'PlatformIO environment native'

$nodeAEnvironment = Get-EnvironmentSection $platformio 'node_a_anchor'
$nodeBEnvironment = Get-EnvironmentSection $platformio 'node_b_tag'
Require-Match $nodeAEnvironment '(?m)^\s*-D\s*BPMWATCH_NODE_ID\s*=\s*0\b' 'node_a_anchor BPMWATCH_NODE_ID=0 build flag'
Require-Match $nodeBEnvironment '(?m)^\s*-D\s*BPMWATCH_NODE_ID\s*=\s*1\b' 'node_b_tag BPMWATCH_NODE_ID=1 build flag'

$sourceFiles = if (Test-Path -LiteralPath $sourceRoot -PathType Container) {
    @(Get-ChildItem -LiteralPath $sourceRoot -File | Where-Object { $_.Extension -in '.h', '.cpp' })
}
else { @() }
$source = ($sourceFiles | ForEach-Object { Get-Content -Raw -LiteralPath $_.FullName }) -join "`n"
$mainPath = Join-Path $sourceRoot 'main.cpp'
$mainSource = if (Test-Path -LiteralPath $mainPath -PathType Leaf) {
    Get-Content -Raw -LiteralPath $mainPath
}
else { '' }
$nodeConfigPath = Join-Path $sourceRoot 'NodeConfig.h'
$nodeConfigSource = if (Test-Path -LiteralPath $nodeConfigPath -PathType Leaf) {
    Get-Content -Raw -LiteralPath $nodeConfigPath
}
else { '' }
$displayPath = Join-Path $sourceRoot 'DiagnosticsDisplay.cpp'
$displaySource = if (Test-Path -LiteralPath $displayPath -PathType Leaf) {
    Get-Content -Raw -LiteralPath $displayPath
}
else { '' }
$uwbPath = Join-Path $sourceRoot 'UwbDiagnostics.cpp'
$uwbSource = if (Test-Path -LiteralPath $uwbPath -PathType Leaf) {
    Get-Content -Raw -LiteralPath $uwbPath
}
else { '' }

Require-Match $nodeConfigSource '(?s)(?:#if|#elif)\s+BPMWATCH_NODE_ID\s*==\s*0\b(?:(?!#elif|#else|#endif).)*0C:8A:D3:7C:E5:A4:00:01' 'NodeConfig ID 0 to EUI 0C:8A:D3:7C:E5:A4:00:01 mapping'
Require-Match $nodeConfigSource '(?s)(?:#if|#elif)\s+BPMWATCH_NODE_ID\s*==\s*1\b(?:(?!#elif|#else|#endif).)*1C:75:C4:F4:E9:D4:00:01' 'NodeConfig ID 1 to EUI 1C:75:C4:F4:E9:D4:00:01 mapping'
Require-Match $source '\bI2C_SPEED_STANDARD\b' 'shared I2C bus uses 100 kHz standard speed for MAX30102/GY-511 compatibility'
Require-Match $source 'kGy511PrimaryAccelAddress\s*=\s*0x19' 'GY-511 primary accelerometer address 0x19'
Require-Match $source 'kGy511FallbackAccelAddress\s*=\s*0x18' 'GY-511 fallback accelerometer address 0x18'
Require-Match $source 'initAccelAt\s*\(\s*kGy511FallbackAccelAddress\s*\)' 'GY-511 accelerometer init falls back to 0x18 when 0x19 fails'
Require-Match $source 'Gy511Status::AccelInitError' 'GY-511 LCD diagnostic distinguishes accelerometer 0x19 init failure'
Require-Match $source 'Gy511Status::MagInitError' 'GY-511 LCD diagnostic distinguishes magnetometer 0x1E init failure'
Require-Match $source 'Gy511Status::MagOnly' 'GY-511 supports magnetometer-only operation when accelerometer is missing'
Require-Match $source 'accelAvailable' 'GY-511 tracks whether accelerometer is available separately from magnetometer heading'
Require-Match $source 'gy511StatusIsInitFailure\s*\(\s*state\.status\s*\)' 'GY-511 sampling preserves detailed init failure label'
Require-Match $source 'Gy511Status::AccelReadError' 'GY-511 LCD diagnostic distinguishes accelerometer 0x19 read failure'
Require-Match $source 'Gy511Status::MagReadError' 'GY-511 LCD diagnostic distinguishes magnetometer 0x1E read failure'
Require-Match $source 'gy511StatusLabel\s*\(\s*state\.status\s*\)' 'GY-511 LCD renders detailed diagnostic status label'
Require-Match $source 'formatI2cAddressList' 'integrated LCD diagnostic formats detected I2C address list'
Require-Match $mainSource 'scanI2cBus\s*\(\s*Wire\s*,\s*state\.gy511\s*\)' 'integrated setup scans I2C bus for LCD diagnostics'
Require-Match $displaySource 'I2C:' 'GY-511 panel displays detected I2C addresses'
Require-Match $source 'struct\s+RadarState' 'RadarMap state for UWB distance-trend radar'
Require-Match $source 'updateRadarState' 'RadarMap trend update function'
Require-Match $source 'radarLinkStatusLabel' 'RadarMap distinguishes WAIT before first range from LOST after timeout'
Require-Match $mainSource 'updateRadarState\s*\(' 'main loop updates radar state from UWB range'
Require-Match $displaySource 'renderRadarMap' 'display renders radar map screen'
Require-Match $displaySource 'previousPeerVisible_' 'radar display redraws only moving peer dot instead of full screen every frame'
Require-Match $uwbSource 'if\s*\(\s*nowMs\s*-\s*lastActivityMs_\s*>=' 'UWB recovery applies to both anchor and tag roles'
Require-Match $source '\besp_now_init\s*\(' 'ESP-NOW range return initializes esp_now'
Require-Match $source '\besp_now_send\s*\(' 'Node A sends range packets over ESP-NOW'
Require-Match $source 'BPMWATCH_TAG_UWB_RECOVERY' 'Node B app-level UWB recovery can remain disabled to match pair test behavior'
Require-Match $source 'xTaskCreatePinnedToCore\s*\(\s*uwbTask' 'ESP-NOW integrated envs can run UWB loop in a dedicated FreeRTOS task'
Require-Match $platformio 'BPMWATCH_UWB_TASK_PRIORITY\s*=\s*3' 'ESP-NOW UWB task uses stable priority 3 to avoid interrupt watchdog resets'
Require-Match $platformio 'BPMWATCH_UWB_TASK_CORE\s*=\s*1' 'ESP-NOW UWB task stays on Arduino core to avoid Wi-Fi/ESP-NOW core contention'
Require-Match $platformio 'UWB_LIBRARY_LOGS\s*=\s*false' 'ESP-NOW normal firmware disables direct DW1000 library boot logs'
Require-Match $source 'DiagnosticsLock' 'shared diagnostic state uses short critical sections across UWB task and main loop'
Require-Match $source 'beginUwbEventQueue' 'UWB task path queues events for main-loop Serial printing'
Require-Match $source 'BPMWATCH_DISCOVERY_DISPLAY_INTERVAL_MS' 'display update cadence slows while UWB has no range'
Require-Match $source 'BPMWATCH_MAX_REINIT_INTERVAL_MS' 'MAX30102 init is retried after transient I2C boot failure'
Require-Match $source 'BPMWATCH_DISPLAY_FULL_REFRESH_MS' 'ST7789 radar display gets periodic full refresh frames'
Require-Match $mainSource 'tryBeginMax30102\s*\(' 'main loop uses MAX30102 retry helper'
Require-Match $mainSource 'Serial\.printf\s*\(\s*"I2C scan:' 'setup prints detected I2C addresses for MAX30102/GY-511 diagnosis'
Require-Match $displaySource 'DISPLAY boot test' 'ST7789 boot path prints display self-test evidence'
Require-Match $displaySource 'ST77XX_RED' 'ST7789 boot path flashes a visible color self-test'
Require-Match $displaySource 'forceFullRefresh' 'display render supports forced full refresh'

if ($source -match '\bportENTER_CRITICAL\b') {
    $errors.Add('FORBIDDEN: DiagnosticsLock must not disable interrupts with portENTER_CRITICAL')
}

$renderRadarBodyMatch = [regex]::Match($displaySource, '(?s)void\s+DiagnosticsDisplay::renderRadarMap\s*\([^)]*\)\s*\{(?<body>.*?)^\}')
if ($renderRadarBodyMatch.Success -and $renderRadarBodyMatch.Groups['body'].Value -match 'fillScreen\s*\(') {
    $errors.Add('FORBIDDEN: renderRadarMap fillScreen blocks UWB ranging loop')
}

$wireBeginCount = ([regex]::Matches($source, 'Wire\s*\.\s*begin\s*\(\s*21\s*,\s*22\s*\)')).Count
if ($wireBeginCount -ne 1) {
    Add-MissingContract "exactly one Wire.begin(21, 22) call (found $wireBeginCount)"
}

$resetPinDefinition = [regex]::Match(
    $source,
    '(?m)(?:#define\s+(?<symbol>kDisplayReset|DISPLAY_RES)\s+27\b|(?:static\s+)?(?:constexpr|const)\s+\w+\s+(?<symbol>kDisplayReset|DISPLAY_RES)\s*=\s*27\s*;)'
)
if (-not $resetPinDefinition.Success) {
    Add-MissingContract 'ST7789 reset pin kDisplayReset or DISPLAY_RES defined as GPIO 27'
    Add-MissingContract 'ordered ST7789 reset using the GPIO 27 reset symbol'
}
else {
    $resetSymbol = [regex]::Escape($resetPinDefinition.Groups['symbol'].Value)
    $resetSequence = "(?s)digitalWrite\s*\(\s*$resetSymbol\s*,\s*HIGH\s*\)\s*;\s*delay\s*\(\s*50\s*\)\s*;.*?digitalWrite\s*\(\s*$resetSymbol\s*,\s*LOW\s*\)\s*;\s*delay\s*\(\s*50\s*\)\s*;.*?digitalWrite\s*\(\s*$resetSymbol\s*,\s*HIGH\s*\)\s*;\s*delay\s*\(\s*50\s*\)\s*;"
    Require-Match $displaySource $resetSequence 'ordered ST7789 reset using the GPIO 27 reset symbol'
}
Require-Match $displaySource '(?s)\.init\s*\(\s*240\s*,\s*240\s*,\s*SPI_MODE3\s*\).*?\.setSPISpeed\s*\(\s*8000000\s*\)' 'ST7789 setSPISpeed(8000000) after init(240, 240, SPI_MODE3)'
Require-Match $uwbSource '\bDW1000Ranging\s*\.\s*loop\s*\(\s*\)\s*;' 'DW1000Ranging.loop() in UwbDiagnostics.cpp'

$loopStart = [regex]::Match($mainSource, '(?s)\bvoid\s+loop\s*\(\s*\)\s*\{')
$pollBeforeScheduler = $false
if ($loopStart.Success) {
    $loopTail = $mainSource.Substring($loopStart.Index + $loopStart.Length)
    $poll = [regex]::Match($loopTail, '\buwb\s*\.\s*poll\s*\(\s*nowMs\s*,\s*state\s*\.\s*uwb\s*\)\s*;')
    $firstScheduler = [regex]::Match($loopTail, '\bif\s*\(\s*\w+\s*-\s*\w+\s*>=')
    $pollBeforeScheduler = $poll.Success -and (-not $firstScheduler.Success -or $poll.Index -lt $firstScheduler.Index)
}
if (-not $pollBeforeScheduler) {
    Add-MissingContract 'uwb.poll(nowMs, state.uwb) before scheduler blocks on every loop pass'
}

Require-Match $mainSource '(?s)sensorIntervalMs\s*=\s*uwbSnapshot\.hasRange\s*\?\s*BPMWATCH_CONNECTED_SENSOR_INTERVAL_MS\s*:\s*BPMWATCH_DISCOVERY_SENSOR_INTERVAL_MS' 'adaptive sensor interval slows MAX30102/GY-511 while UWB has no range'
Require-Match $mainSource '(?s)if\s*\(\s*\w+\s*-\s*\w+\s*>=\s*sensorIntervalMs\s*\)\s*\{[^}]*\bmax30102\s*\.\s*sample\s*\(' 'elapsed subtraction scheduler calling max30102.sample with adaptive interval'
Require-Match $mainSource '(?s)if\s*\(\s*\w+\s*-\s*\w+\s*>=\s*sensorIntervalMs\s*\)\s*\{[^}]*\bgy511\s*\.\s*sample\s*\(' 'elapsed subtraction scheduler calling gy511.sample with adaptive interval'
Require-Match $mainSource '(?s)displayIntervalMs\s*=\s*uwbSnapshot\.hasRange\s*\?\s*BPMWATCH_CONNECTED_DISPLAY_INTERVAL_MS\s*:\s*BPMWATCH_DISCOVERY_DISPLAY_INTERVAL_MS' 'adaptive display interval slows ST7789 rendering while UWB has no range'
Require-Match $mainSource '(?s)if\s*\(\s*\w+\s*-\s*\w+\s*>=\s*displayIntervalMs\s*\).*?\bdisplay\s*\.\s*render\s*\(' 'elapsed subtraction scheduler calling display.render with adaptive interval'
Require-Match $mainSource '(?s)if\s*\(\s*\w+\s*-\s*\w+\s*>=\s*1000(?:U|UL|L)?\s*\)\s*\{[^}]*\blogDiagnostics\s*\(' 'elapsed subtraction >= 1000ms scheduler calling logDiagnostics'

$forbiddenPatterns = [ordered]@{
    'BluetoothSerial' = '\bBluetoothSerial\b'
    'esp_spp'         = '\besp_spp\w*\b'
    'BLEDevice'       = '\bBLEDevice\b'
    'WebServer.h'     = 'WebServer.h'
    'softAP'          = '\bsoftAP\b'
    'delay(20)'       = 'delay\s*\(\s*20\s*\)'
    'I2C_SPEED_FAST'  = '\bI2C_SPEED_FAST\b'
}
foreach ($entry in $forbiddenPatterns.GetEnumerator()) {
    if ($source -match $entry.Value) {
        $errors.Add("FORBIDDEN: $($entry.Key)")
    }
}

if ($errors.Count -gt 0) {
    $errors | ForEach-Object { Write-Output $_ }
    exit 1
}

Write-Output 'PASS: integrated node diagnostics contract verified.'
