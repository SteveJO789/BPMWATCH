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
    'src/MedianRangeFilter.h'
    'src/Gy511Sensor.h'
    'src/Gy511Sensor.cpp'
    'src/Max30102Sensor.h'
    'src/Max30102Sensor.cpp'
    'src/UwbDiagnostics.h'
    'src/UwbDiagnostics.cpp'
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
Require-Match $platformio '(?m)^\s*\[env:node_b_tag\]\s*$' 'PlatformIO environment node_b_tag'
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

$schedulerContracts = @(
    @{ Interval = 20; Call = '\bmax30102\s*\.\s*sample\s*\('; Description = 'max30102.sample' }
    @{ Interval = 50; Call = '\bgy511\s*\.\s*sample\s*\('; Description = 'gy511.sample' }
    @{ Interval = 200; Call = '\bdisplay\s*\.\s*render\s*\('; Description = 'display.render' }
    @{ Interval = 1000; Call = '\blogDiagnostics\s*\('; Description = 'logDiagnostics' }
)
foreach ($contract in $schedulerContracts) {
    $interval = $contract.Interval
    $pattern = "(?s)if\s*\(\s*\w+\s*-\s*\w+\s*>=\s*$interval(?:U|UL|L)?\s*\)\s*\{[^}]*$($contract.Call)"
    Require-Match $mainSource $pattern "elapsed subtraction >= ${interval}ms scheduler calling $($contract.Description)"
}

$forbiddenPatterns = [ordered]@{
    'BluetoothSerial' = '\bBluetoothSerial\b'
    'esp_spp'         = '\besp_spp\w*\b'
    'BLEDevice'       = '\bBLEDevice\b'
    'WiFi.h'          = 'WiFi.h'
    'WebServer.h'     = 'WebServer.h'
    'softAP'          = '\bsoftAP\b'
    'delay(20)'       = 'delay\s*\(\s*20\s*\)'
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
