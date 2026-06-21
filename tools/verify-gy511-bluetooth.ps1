$ErrorActionPreference = 'Stop'

$sourcePath = Join-Path $PSScriptRoot '..\firmware\tests\gy511_test\src\main.cpp'
$source = Get-Content -Raw -Path $sourcePath

$checks = [ordered]@{
    'BluetoothSerial include' = '#include <BluetoothSerial.h>'
    'Bluetooth serial instance' = 'BluetoothSerial SerialBT;'
    'Bluetooth device name' = 'SerialBT.begin("BPMWATCH-GY511")'
    'SSP just-works configuration' = 'SerialBT.enableSSP(false, false);'
    'Bluetooth readiness guard' = 'if (bluetoothReady)'
    'Shared line output helper' = 'void logLine(const char* message)'
    'USB output in helper' = 'Serial.println(message);'
    'Bluetooth output in helper' = 'SerialBT.println(message);'
}

$missing = @()
foreach ($check in $checks.GetEnumerator()) {
    if (-not $source.Contains($check.Value)) {
        $missing += $check.Key
    }
}

if ($missing.Count -gt 0) {
    foreach ($name in $missing) {
        Write-Error "Missing contract: $name" -ErrorAction Continue
    }
    exit 1
}

$sspIndex = $source.IndexOf('SerialBT.enableSSP(false, false);')
$beginIndex = $source.IndexOf('SerialBT.begin("BPMWATCH-GY511")')
if ($sspIndex -gt $beginIndex) {
    Write-Error 'SSP must be enabled before BluetoothSerial.begin().' -ErrorAction Continue
    exit 1
}

if ($source.Contains('SerialBT.setPin(')) {
    Write-Error 'Legacy Bluetooth PIN configuration must not be present.' -ErrorAction Continue
    exit 1
}

Write-Output 'PASS: GY-511 Bluetooth diagnostic contract verified.'
